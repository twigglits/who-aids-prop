#include "eventdvr.h"
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

bool EventDVR::m_DVR_enabled = true;
double EventDVR::s_DVRThreshold = 0.5;

EventDVR::EventDVR(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventDVR::~EventDVR()
{
}

// rest of our template functions
string EventDVR::getDescription(double tNow) const
{
	return strprintf("DVR event for %s", getPerson(0)->getName().c_str());
}

void EventDVR::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventDVR::isEligibleForTreatment(double t, const State *pState, Person *pPerson)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    if (pPerson->isWoman() && !WOMAN(pPerson)->isDVR() && !pPerson->hiv().isInfected() && !pPerson->isPrep()){
        return true;
    }else{
        return false;
    }
}

bool EventDVR::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson) {
    assert(m_DVRprobDist);
    double dt = m_DVRprobDist->pickNumber();

    if(dt > s_DVRThreshold){
        return true;
    }
    return false;
}

double EventDVR::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    double dt = 1.0;
    return dt;
}

void EventDVR::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);

    if (m_DVR_enabled){
        if (isEligibleForTreatment(t, pState, pPerson) && isWillingToStartTreatment(t, pRndGen, pPerson)) 
        {
        WOMAN(pPerson)->setDVR(true);
        std::cout << "DVR FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
        writeEventLogStart(true, "DVR_treatment", t, pPerson, 0);
        }
    }
}

ProbabilityDistribution *EventDVR::m_DVRprobDist = 0;


void EventDVR::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Process DRV probability distribution
    if (m_DVRprobDist) {
        delete m_DVRprobDist;
        m_DVRprobDist = 0;
    }
    m_DVRprobDist = getDistributionFromConfig(config, pRndGen, "EventDVR.m_DVRprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventDVR.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventDVR.threshold", s_DVRThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_DVR_enabled = (enabledStr == "true");

}

void EventDVR::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventDVR.enabled", m_DVR_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventDVR.threshold", s_DVRThreshold))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_DVRprobDist, config, "EventDVR.m_DVRprobDist");
}

ConfigFunctions DVRConfigFunctions(EventDVR::processConfig, EventDVR::obtainConfig, "EventDVR");

JSONConfig DVRJSONConfig(R"JSON(
    "eventDVR": { 
        "depends": null,
        "params": [
            ["EventDVR.enabled", "true", [ "true", "false"] ],
            ["EventDVR.threshold", 0.5],
            ["EventDVR.m_DVRprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    },
    "eventDVR_schedule_dist": { 
        "depends": null,
        "params": [  
            [ "eventDVR.m_prepscheduleDist.dist", "distTypes", ["fixed", [ ["value", 0.246575 ] ] ] ] 
        ],
        "info": [ 
            "This parameter is used to specify the VMMC scheduling duration. The default is fixed."
        ]
    }
)JSON");