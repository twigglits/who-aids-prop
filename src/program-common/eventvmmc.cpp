#include "eventvmmc.h"
#include "gslrandomnumbergenerator.h"
#include "configdistributionhelper.h"
#include "util.h"
#include "configsettings.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configsettingslog.h"
#include <iostream>
#include <cstdlib> // for rand() function
#include <chrono>

using namespace std;

EventVMMC::EventVMMC(Person *pMan) : SimpactEvent(pMan)
{
	assert(pMan->isMan());
}

EventVMMC::~EventVMMC()
{
}

double EventVMMC::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);

	double evtTime = getNextInterventionTime();
	double dt = evtTime - population.getTime();

	if (evtTime < 0 || dt < 0)
		abortWithMessage("EventVMMC::getNextInterventionTime: the next VMMC intervention takes place at time " + doubleToString(evtTime) + " which is before the current time " + doubleToString(population.getTime()) + " (dt = " + doubleToString(dt) + ")"); 
	
	return dt;
}

string EventVMMC::getDescription(double tNow) const
{
	return strprintf("VMMC event for %s", getPerson(0)->getName().c_str());
}

void EventVMMC::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pMan = getPerson(0);
	writeEventLogStart(true, "VMMC", tNow, pMan, 0);  //when set to true we write logs
}

bool EventVMMC::isEligibleForTreatment(double t)
{
    // Person must be male and 15 years or older to be eligible for treatment.
    Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());   // we assert that a person is from the male class
    
    double age = pMan->getAgeAt(t);
    if (pMan->isMan() && !pMan->isVmmc() && age >= 15.0) {  //if person is male & not yet circumsized & age 15 or older
        return true;  // eligible for treatment
    } else {
        return false; // not eligible for treatment
    }
}

bool EventVMMC::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
	Person *pMan = getPerson(0);

	double x = pRndGen->pickRandomDouble();
	if (x < pMan->hiv().getARTAcceptanceThreshold())  //sampled from fixed distribution, which is a coin toss TODO: duplicate so that it is uncoupled from hivARTacceptanceThreshold
		return true;

	return false;
}

void EventVMMC::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	double interventionTime;
	ConfigSettings interventionConfig;

	popNextInterventionInfo(interventionTime, interventionConfig);
	assert(interventionTime == t); // make sure we're at the correct time
	
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pMan = getPerson(0);
	
	if (isEligibleForTreatment(t) && isWillingToStartTreatment(t, pRndGen))  //so if this condition is met we set the VMMC property to true
	{
		SimpactEvent::writeEventLogStart(true, "(VMMC_treatment)", t, pMan, 0);

		Man *pMan = MAN(getPerson(0));
        assert(pMan->isMan()); // Ensure the person is a man
        
        // Set the isVmmc property to true
        pMan->setVmmc(true);
	}
	population.initializeFormationEvents(pMan, false, false, t);
}

// double EventVMMC:: = -1;
// PieceWiseLinearFunction *EventVMMC::s_pRecheckInterval = 0;

void EventVMMC::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{

	// m_pPregDurationDist = getDistributionFromConfig(config, pRndGen, "birth.pregnancyduration");

	bool_t r;
	// if (!(r = config.getKeyValue("birth.boygirlratio", m_boyGirlRatio, 0, 1)))
		// abortWithMessage(r.getErrorString());
}

void EventVMMC::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	// addDistributionToConfig(m_pPregDurationDist, config, "birth.pregnancyduration");
	// if (!(r = config.addKey("birth.boygirlratio", m_boyGirlRatio)))
	// 	abortWithMessage(r.getErrorString());
}

ConfigFunctions birthConfigFunctions(EventVMMC::processConfig, EventVMMC::obtainConfig, "EventVMMC");

// JSONConfig birthJSONConfig(R"JSON(
//         "EventBirth": {
//             "depends": null,
//             "params": [ ["birth.boygirlratio", 0.49751243781094534 ] ],
//             "info": [
//                 "When someone is born, a random number is chosen from [0,1],",
//                 "and if smaller than this boygirlratio, the new child is male. Otherwise, a ",
//                 "woman is added to the population.",
//                 "",
//                 "Default is 1.0/2.01"
//             ]
//         },

//         "EventBirth_pregduration": { 
//             "depends": null,
//             "params": [ [ 
//                 "birth.pregnancyduration.dist", "distTypes", ["fixed", [ ["value", 0.7342465753424657 ] ] ] 
//                 ] 
//             ],
//             "info": [ 
//                 "This parameter is used to specify the pregnancy duration. The default",
//                 "is the fixed value of 268/365"
//             ]
//         })JSON");