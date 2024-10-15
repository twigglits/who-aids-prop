#include "eventprepdrop.h"
#include "eventdiagnosis.h"
#include "configsettings.h"
#include "configwriter.h"  
#include "eventdvrdrop.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

double EventPrepDrop::s_prepdropThreshold = 0.2;

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
	// writeEventLogStart(true, "prepdrop", tNow, pPerson, 0);
}

bool EventPrepDrop::dropOutFraction(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_prepDropDistribution);
	double dt = m_prepDropDistribution->pickNumber();
    if (dt > s_prepdropThreshold){
        return true;
    }
    return false;
}

void EventPrepDrop::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pPerson = getPerson(0);
    if ((dropOutFraction(t, pRndGen) && pPerson->isPrep()) || (pPerson->getNumberOfRelationships()==0 && pPerson->isPrep()) || (pPerson->hiv().isInfected() && pPerson->isPrep()))
    {
    pPerson->setPrep(false);
    writeEventLogStart(true, "PrepDrop", t, pPerson, 0);
    }
}

double EventPrepDrop::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
    dt = 0.3;
	return dt;
}

ProbabilityDistribution *EventPrepDrop::m_prepDropDistribution = 0;

void EventPrepDrop::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
    bool_t r;
    if (m_prepDropDistribution){
        delete m_prepDropDistribution;
        m_prepDropDistribution = 0;
    }
	m_prepDropDistribution = getDistributionFromConfig(config, pRndGen, "EventPrepDrop.interval");

    if (!(r = config.getKeyValue("EventPrepDrop.threshold", s_prepdropThreshold))){
        abortWithMessage(r.getErrorString());}
}

void EventPrepDrop::obtainConfig(ConfigWriter &config)
{
    bool_t r;
    if (!(r = config.addKey("EventPrepDrop.threshold", s_prepdropThreshold))) {
        abortWithMessage(r.getErrorString());
    }
	addDistributionToConfig(m_prepDropDistribution, config, "EventPrepDrop.interval");
}

ConfigFunctions prepdropConfigFunctions(EventPrepDrop::processConfig, EventPrepDrop::obtainConfig, "EventPrepDrop");

JSONConfig prepdropJSONConfig(R"JSON(
        "EventPrepDrop_Timing": {
            "depends": null,
            "params": [ 
                ["EventPrepDrop.threshold", 0.2],
                [ "EventPrepDrop.interval.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
            ],
            "info": [
                "Distribution to schedule dropout of prep intervention."
            ]
        })JSON");