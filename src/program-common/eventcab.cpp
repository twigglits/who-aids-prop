#include "eventcab.h"
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

bool EventCAB::m_CAB_enabled = true;
double EventCAB::s_CABThreshold = 0.5;

EventCAB::EventCAB(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventCAB::~EventCAB()
{
}

// rest of our template functions
string EventCAB::getDescription(double tNow) const
{
	return strprintf("CAB event for %s", getPerson(0)->getName().c_str());
}

void EventCAB::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventCAB::isEligibleForTreatment(double t, const State *pState, Person *pPerson)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    if (pPerson->isWoman()){
        if (!pPerson->hiv().isInfected() && !pPerson->isPrep() && !WOMAN(pPerson)->isDVR() && !pPerson->isCAB()){
            return true;
        }
        return false;
    }else{
        if(!pPerson->hiv().isInfected() && !pPerson->isPrep() && !pPerson->isCAB()){
            return true;
        }
        return false;
    }
    return false;
}

bool EventCAB::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson) {
    assert(m_CABprobDist);
    double dt = m_CABprobDist->pickNumber();

    if(dt > s_CABThreshold){
        return true;
    }
    return false;
}

double EventCAB::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    double dt = 0.0000001;
    return dt;
}

void EventCAB::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);

    if (m_CAB_enabled){
        if (isEligibleForTreatment(t, pState, pPerson) && isWillingToStartTreatment(t, pRndGen, pPerson)) 
        {
        pPerson->setCAB(true);
        std::cout << "CAB FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
        writeEventLogStart(true, "CAB_treatment", t, pPerson, 0);
        }
    }
}

ProbabilityDistribution *EventCAB::m_CABprobDist = 0;


void EventCAB::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Process DRV probability distribution
    if (m_CABprobDist) {
        delete m_CABprobDist;
        m_CABprobDist = 0;
    }
    m_CABprobDist = getDistributionFromConfig(config, pRndGen, "EventCAB.m_CABprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventCAB.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventCAB.threshold", s_CABThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_CAB_enabled = (enabledStr == "true");

}

void EventCAB::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventCAB.enabled", m_CAB_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventCAB.threshold", s_CABThreshold))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_CABprobDist, config, "EventCAB.m_CABprobDist");
}

ConfigFunctions CABConfigFunctions(EventCAB::processConfig, EventCAB::obtainConfig, "EventCAB");

JSONConfig CABJSONConfig(R"JSON(
    "eventCAB": { 
        "depends": null,
        "params": [
            ["EventCAB.enabled", "true", [ "true", "false"] ],
            ["EventCAB.threshold", 0.5],
            ["EventCAB.m_CABprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept CAB treatment",
            "and to enable or disable the CAB event."
        ]
    }
)JSON");