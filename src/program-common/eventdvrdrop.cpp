#include "eventdvrdrop.h"
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

bool EventDVRDROP::m_DVRDROP_enabled = true;
double EventDVRDROP::s_DVRDROPThreshold = 0.5;

EventDVRDROP::EventDVRDROP(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventDVRDROP::~EventDVRDROP()
{
}

string EventDVRDROP::getDescription(double tNow) const
{
	return strprintf("DVRDROP event for %s", getPerson(0)->getName().c_str());
}

void EventDVRDROP::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventDVRDROP::isEligibleForTreatment(double t, const State *pState, Person *pPerson){
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    if (pPerson->isWoman() && WOMAN(pPerson)->isDVR()){
        return true;
    }
    return false;
}

bool EventDVRDROP::isHardDropOut(double t, const State *pState, Person *pPerson){
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    if ((pPerson->isWoman() && WOMAN(pPerson)->isDVR()) && (pPerson->hiv().isInfected() || pPerson->isPrep() || pPerson->getNumberOfRelationships()==0 || pPerson->isCAB()))  // here we check age and woman again, if person is infected with HIV we drop out, if person is on Oral prep we also drop out
        {
            return true;
        }
    return false;
}

bool EventDVRDROP::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson){ 
    assert(m_DVRDROPprobDist);
    double dt = m_DVRDROPprobDist->pickNumber();
    if(dt > s_DVRDROPThreshold){
        return true;
    }
    return false;
}

double EventDVRDROP::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    assert(m_DVRDROPscheduleDist);
    double dt = m_DVRDROPscheduleDist->pickNumber();
	return dt;
}


void EventDVRDROP::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);

    if (m_DVRDROP_enabled){

        if (isHardDropOut(t, pState, pPerson))
        {
            WOMAN(pPerson)->setDVR(false);
            std::cout << "DVR_DROP_HARD FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
            writeEventLogStart(true, "DVR_DROP", t, pPerson, 0);
        }

        if (isEligibleForTreatment(t, pState, pPerson) && (isWillingToStartTreatment(t, pRndGen, pPerson)))  
        {
            WOMAN(pPerson)->setDVR(false);
            std::cout << "DVR_DROP_SOFT FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
            writeEventLogStart(true, "DVR_DROP", t, pPerson, 0);
        }
    }
}

ProbabilityDistribution *EventDVRDROP::m_DVRDROPprobDist = 0;
ProbabilityDistribution *EventDVRDROP::m_DVRDROPscheduleDist = 0;

void EventDVRDROP::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    if (m_DVRDROPprobDist) {
        delete m_DVRDROPprobDist;
        m_DVRDROPprobDist = 0;
    }
    m_DVRDROPprobDist = getDistributionFromConfig(config, pRndGen, "EventDVRDROP.m_DVRDROPprobDist");

    if (m_DVRDROPscheduleDist) {
        delete m_DVRDROPscheduleDist;
        m_DVRDROPscheduleDist = 0;
    }
    m_DVRDROPscheduleDist = getDistributionFromConfig(config, pRndGen, "EventDVRDROP.m_DVRDROPscheduleDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventDVRDROP.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventDVRDROP.threshold", s_DVRDROPThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_DVRDROP_enabled = (enabledStr == "true");
}

void EventDVRDROP::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventDVRDROP.enabled", m_DVRDROP_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventDVRDROP.threshold", s_DVRDROPThreshold))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_DVRDROPprobDist, config, "EventDVRDROP.m_DVRDROPprobDist");
    addDistributionToConfig(m_DVRDROPscheduleDist, config, "EventDVRDROP.m_DVRDROPscheduleDist");
}

ConfigFunctions DVRDROPConfigFunctions(EventDVRDROP::processConfig, EventDVRDROP::obtainConfig, "EventDVRDROP");

JSONConfig DVRDROPJSONConfig(R"JSON(
    "EventDVRDROP": { 
        "depends": null,
        "params": [
            ["EventDVRDROP.enabled", "true", [ "true", "false"] ],
            ["EventDVRDROP.threshold", 0.5],
            ["EventDVRDROP.m_DVRDROPscheduleDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ] ],
            ["EventDVRDROP.m_DVRDROPprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    }
)JSON");