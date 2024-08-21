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
#include <cstdlib>
#include <chrono>

using namespace std;

bool EventPrep::m_prep_enabled = true;
double EventPrep::s_prepThreshold = 0.5;
double EventPrep::s_prepThresholdAGYW = 0.5;

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

bool EventPrep::isEligibleForTreatment(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    Person *pPerson1 = getPerson(0);

    if (!pPerson1->hiv().isInfected() && !pPerson1->isPrep()){
        // std::cout << "Person:" << pPerson1->getName() << "is eligible for treatment Prep" <<  std::endl;
        return true;
    }else{
        // std::cout << "Person:" << pPerson1->getName() << "is NOT eligible for treatment: Prep" << std::endl;
        return false;
    }
}

bool EventPrep::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, const State *pState) {
    assert(m_prepprobDist);
    Person *pPerson1 = getPerson(0);
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double curTime = population.getTime();
    double age = pPerson1->getAgeAt(curTime); 
    double dt = m_prepprobDist->pickNumber();

    if (pPerson1->isWoman() && age >= 15 && age < 25){
        if (dt > s_prepThresholdAGYW) {
            return true;
        }else{
            return false;
        }
    }
    else{
        if (dt > s_prepThreshold){
            return true;
        }
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
    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    ConfigSettings interventionConfig;

    Person *pPerson1 = getPerson(0);
    double interventionTime;
    assert(interventionTime == t); 

    if (isEligibleForTreatment(t, pState) && isWillingToStartTreatment(t, pRndGen, pState)) 
    {
    pPerson1->setPrep(true);
    std::cout << "Person:" << pPerson1->getName() << "with Gender:" << pPerson1->getGender() << "is eligible for treatment Prep" <<  std::endl;
    writeEventLogStart(true, "Prep_Treatment", t, pPerson1, 0);
    
	EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson1, t);
	population.onNewEvent(pEvtPrepDrop);
    }
}

ProbabilityDistribution *EventPrep::m_prepprobDist = 0;
ProbabilityDistribution *EventPrep::m_prepscheduleDist = 0;
PieceWiseLinearFunction *EventPrep::s_pRecheckInterval = 0;

void EventPrep::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;
    
    
    if (m_prepscheduleDist) {
        delete m_prepscheduleDist;
        m_prepscheduleDist = 0;
    }
    m_prepscheduleDist = getDistributionFromConfig(config, pRndGen, "EventPrep.m_prepscheduleDist");

    
    if (m_prepprobDist) {
        delete m_prepprobDist;
        m_prepprobDist = 0;
    }
    m_prepprobDist = getDistributionFromConfig(config, pRndGen, "EventPrep.m_prepprobDist");

    
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventPrep.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventPrep.threshold", s_prepThreshold)) ||
        !(r = config.getKeyValue("EventPrep.thresholdAGYW", s_prepThresholdAGYW))){
        abortWithMessage(r.getErrorString());
    }
    m_prep_enabled = (enabledStr == "true");

}

void EventPrep::obtainConfig(ConfigWriter &config) {
    bool_t r;

    
    if (!(r = config.addKey("EventPrep.enabled", m_prep_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventPrep.threshold", s_prepThreshold)) ||
        !(r = config.addKey("EventPrep.thresholdAGYW", s_prepThresholdAGYW))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_prepscheduleDist, config, "EventPrep.m_prepscheduleDist");
    addDistributionToConfig(m_prepprobDist, config, "EventPrep.m_prepprobDist");
}

ConfigFunctions PrepConfigFunctions(EventPrep::processConfig, EventPrep::obtainConfig, "EventPrep");

JSONConfig PrepJSONConfig(R"JSON(
    "EventPrep": { 
        "depends": null,
        "params": [
            ["EventPrep.enabled", "true", [ "true", "false"] ],
            ["EventPrep.threshold", 0.5],
            ["EventPrep.thresholdAGYW", 0.5],
            ["EventPrep.m_prepprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    },
    "EventPrep_schedule_dist": { 
        "depends": null,
        "params": [  
            [ "EventPrep.m_prepscheduleDist.dist", "distTypes", ["fixed", [ ["value", 0.246575 ] ] ] ] 
        ],
        "info": [ 
            "This parameter is used to specify the VMMC scheduling duration. The default is fixed."
        ]
    }
)JSON");
