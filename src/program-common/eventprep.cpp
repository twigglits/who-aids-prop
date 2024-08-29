#include "eventprep.h"
#include "gslrandomnumbergenerator.h"
#include "configdistributionhelper.h"
#include "util.h"
#include "configsettings.h"
#include "eventprepdrop.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configsettingslog.h"
#include <iostream>
#include <cstdlib> // for rand() function
#include <chrono>

using namespace std;

bool EventPrep::m_prep_enabled = true;
double EventPrep::s_prepThreshold = 0.5;
double EventPrep::s_AGYWThreshold = 0.5;

EventPrep::EventPrep(Person *pPerson1, bool scheduleImmediately) : SimpactEvent(pPerson1)
{
}

EventPrep::~EventPrep()
{
}

string EventPrep::getDescription(double tNow) const
{
	return strprintf("Prep event for %s", getPerson(0)->getName().c_str());
}

void EventPrep::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventPrep::isEligibleForTreatmentP1(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    Person *pPerson1 = getPerson(0);

    if (!pPerson1->hiv().isInfected() && !pPerson1->isPrep()){
        return true;
    }else{
        return false;
    }
}

bool EventPrep::isWillingToStartTreatmentP1(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_prepprobDist);
    Person *pPerson1 = getPerson(0);
    double dt = m_prepprobDist->pickNumber();
    if (pPerson1->isWoman() && WOMAN(pPerson1)->isAGYW() && dt > s_AGYWThreshold)
    {
        return true;
    }
    else if (dt > s_prepThreshold)
    {
        return true;
    }else{
        return false;
    }
}

double EventPrep::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    double dt = 1.0;
    return dt;
}


void EventPrep::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson1 = getPerson(0);
    double curTime = population.getTime();
    double age1 = pPerson1->getAgeAt(curTime);
    assert(interventionTime == t); 

    if (isEligibleForTreatmentP1(t, pState) && isWillingToStartTreatmentP1(t, pRndGen)) 
    {
    pPerson1->setPrep(true);
    writeEventLogStart(true, "Prep_treatment_P1", t, pPerson1, 0);
    
	EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson1, t);
	population.onNewEvent(pEvtPrepDrop);
    }
}

ProbabilityDistribution *EventPrep::m_prepprobDist = 0;
PieceWiseLinearFunction *EventPrep::s_pRecheckInterval = 0;

void EventPrep::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    if (m_prepprobDist) {
        delete m_prepprobDist;
        m_prepprobDist = 0;
    }
    m_prepprobDist = getDistributionFromConfig(config, pRndGen, "EventPrep.m_prepprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventPrep.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventPrep.threshold", s_prepThreshold)) ||
        !(r = config.getKeyValue("EventPrep.AGYWThreshold", s_AGYWThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_prep_enabled = (enabledStr == "true");

}

void EventPrep::obtainConfig(ConfigWriter &config) {
    bool_t r;

    
    if (!(r = config.addKey("EventPrep.enabled", m_prep_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventPrep.threshold", s_prepThreshold)) || 
        !(r = config.addKey("EventPrep.AGYWThreshold", s_AGYWThreshold))){
            abortWithMessage(r.getErrorString());
        }

    addDistributionToConfig(m_prepprobDist, config, "EventPrep.m_prepprobDist");
}

ConfigFunctions PrepConfigFunctions(EventPrep::processConfig, EventPrep::obtainConfig, "EventPrep");

JSONConfig PrepJSONConfig(R"JSON(
    "EventPrep": { 
        "depends": null,
        "params": [
            ["EventPrep.enabled", "true", [ "true", "false"] ],
            ["EventPrep.threshold", 0.5],
            ["EventPrep.AGYWThreshold", 0.5],
            ["EventPrep.m_prepprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    }
)JSON");
