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
    assert(pPerson->getNumberOfRelationships()>0);
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

void EventPrepDrop::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);
    std::cout << "Before PrepDrop status: " << pPerson->isPrep() << " for: " << pPerson->getName() << std::endl;
    pPerson->setPrep(false);
    writeEventLogStart(true, "prepdrop", t, pPerson, 0);
    std::cout << "After PrepDrop status: " << pPerson->isPrep() << " for: " << pPerson->getName() << std::endl;
	
}

double EventPrepDrop::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	// TODO: this is just a temporaty solution, until a real hazard has been defined
	
	int count = 0;
	int maxCount = 1024;
	double dt = -1;

	assert(s_prepDropDistribution);

	while (dt < 0 && count++ < maxCount)
		dt = s_prepDropDistribution->pickNumber();

	if (dt < 0)
		abortWithMessage("EventPrepDrop: couldn't find a positive time interval for next event");

	return dt;
}

double EventPrepDrop::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO: this will need to change when using a real hazard
	return dt;
}

double EventPrepDrop::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO: this will need to change when using a real hazard
	return Tdiff;
}

ProbabilityDistribution *EventPrepDrop::s_prepDropDistribution = 0;

void EventPrepDrop::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	delete s_prepDropDistribution;
	s_prepDropDistribution = getDistributionFromConfig(config, pRndGen, "prepdrop.interval");
}

void EventPrepDrop::obtainConfig(ConfigWriter &config)
{
	addDistributionToConfig(s_prepDropDistribution, config, "prepdrop.interval");
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

