#include "eventstudystep.h"
#include "eventstudyend.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "maxartpopulation.h"
#include "facilities.h"
#include "util.h"
#include <vector>

using namespace std;

EventStudyStep::EventStudyStep(int stepIdx)
{
	assert(stepIdx >= 0);
	m_stepIndex = stepIdx;
}

EventStudyStep::~EventStudyStep()
{
}

double EventStudyStep::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(s_stepInterval >= 0);
	return s_stepInterval;
}

std::string EventStudyStep::getDescription(double tNow) const
{
	return "Study step";
}

void EventStudyStep::writeLogs(const Population &pop, double tNow) const
{
	writeEventLogStart(true, "studystep", tNow, 0, 0);
}

void EventStudyStep::fire(State *pState, double t)
{
	MaxARTPopulation &population = MAXARTPOPULATION(pState);
	assert(population.getStudyStage() == MaxARTPopulation::InStudy);

	Facilities *pFacilities = Facilities::getInstance();
	assert(pFacilities);

	int numRandSteps = pFacilities->getNumberOfRandomizationSteps();

	// Advance previous facilities to intervention stage
	if (m_stepIndex > 0)
	{
		vector<Facility *> f;

		assert(m_stepIndex-1 < numRandSteps);
		pFacilities->getFacilitiesForRandomizationStep(m_stepIndex-1, f);

		for (size_t i = 0 ; i < f.size() ; i++)
		{
			Facility *pFacility = f[i];
			assert(pFacility && pFacility->getStage() == Facility::TransitionStage);

			pFacility->advanceStage();
			assert(pFacility->getStage() == Facility::InterventionStage);
		}
	}

	if (m_stepIndex < numRandSteps) // New facilities are moving to transition stage
	{
		EventStudyStep *pEvt = new EventStudyStep(m_stepIndex+1);
		population.onNewEvent(pEvt);

		// Advance current facilities in transition stage
		vector<Facility *> f;

		pFacilities->getFacilitiesForRandomizationStep(m_stepIndex, f);
		for (size_t i = 0 ; i < f.size() ; i++)
		{
			Facility *pFacility = f[i];
			assert(pFacility && pFacility->getStage() == Facility::ControlStage);

			pFacility->advanceStage();
			assert(pFacility->getStage() == Facility::TransitionStage);
		}
	}
	else
	{
		assert(m_stepIndex == numRandSteps); // End of last facilities in transition stage

		EventStudyEnd *pEvt = new EventStudyEnd();
		population.onNewEvent(pEvt);
	}

	//pFacilities->dump();
}

double EventStudyStep::s_stepInterval = -1;

void EventStudyStep::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (!config.getKeyValue("maxart.stepinterval", s_stepInterval, 0))
		abortWithMessage(config.getErrorString());
}

void EventStudyStep::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("maxart.stepinterval", s_stepInterval))
		abortWithMessage(config.getErrorString());
}

ConfigFunctions studyStepConfigFunctions(EventStudyStep::processConfig, EventStudyStep::obtainConfig, "EventStudyStep");

JSONConfig studyStepJSONConfig(R"JSON(
        "EventStudyStep": { 
            "depends": null, 
            "params" : [ [ "maxart.stepinterval", 0.3333333333333 ] ],
            "info": [
                "The time interval between steps in the MaxART study, defaults to 4 months"
            ]
        })JSON");

