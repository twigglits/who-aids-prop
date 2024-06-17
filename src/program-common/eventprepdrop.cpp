#include "eventprepdrop.h"
#include "eventdiagnosis.h"
#include "configsettings.h"
#include "configwriter.h"           
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventPrepDrop::EventPrepDrop(Person *pPerson, double t) : SimpactEvent(pPerson)
{
    assert(pPerson->isPrep());
}

EventPrepDrop::~EventPrepDrop()
{
}

string EventPrepDrop::getDescription(double tNow) const
{
	return strprintf("PrepDrop event for %s", getPerson(0)->getName().c_str());
}

void EventPrepDrop::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "prepdrop", tNow, pPerson, 0);
}

bool EventPrepDrop::dropOutFraction(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_prepDropDistribution);
	double dt = m_prepDropDistribution->pickNumber();
    if (dt > 0.2){
        return true;
    }
    return false;
}

void EventPrepDrop::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pPerson = getPerson(0);
    if ((dropOutFraction(t, pRndGen) && pPerson->isPrep()) || (pPerson->getNumberOfRelationships()==0) || (pPerson->hiv().isInfected())) 
    {
    pPerson->setPrep(false);
    writeEventLogStart(true, "PrepDrop", t, pPerson, 0);
    std::cout << "After PrepDrop status: " << pPerson->isPrep() << " for: " << pPerson->getName() << std::endl;  //maybe should change to getPrep?
    }
}

double EventPrepDrop::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	return dt;
}

double EventPrepDrop::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	return Tdiff;
}

ProbabilityDistribution *EventPrepDrop::m_prepDropDistribution = 0;

void EventPrepDrop::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	delete m_prepDropDistribution;
	m_prepDropDistribution = getDistributionFromConfig(config, pRndGen, "prepdrop.interval");
}

void EventPrepDrop::obtainConfig(ConfigWriter &config)
{
	addDistributionToConfig(m_prepDropDistribution, config, "prepdrop.interval");
}

ConfigFunctions prepdropConfigFunctions(EventPrepDrop::processConfig, EventPrepDrop::obtainConfig, "EventPrepDrop");

JSONConfig prepdropJSONConfig(R"JSON(
        "EventPrepDrop_Timing": {
            "depends": null,
            "params": [ 
                [ "prepdrop.interval.dist", "distTypes", [ "uniform", [ [ "min", 0.25  ], [ "max", 10.0 ] ] ] ]
            ],
            "info": [
                "Distribution to schedule dropout of prep intervention."
            ]
        })JSON");