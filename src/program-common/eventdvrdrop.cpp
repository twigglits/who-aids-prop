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
int EventDVRDROP::s_DVRDROPScheduleMax = 5;
int EventDVRDROP::s_DVRDROPScheduleMin = 1;

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
    }else{
        return false;
    }
    return false;
}

bool EventDVRDROP::isHardDropOut(double t, const State *pState, Person *pPerson){
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    if (pPerson->isWoman() && WOMAN(pPerson)->isDVR() && ( pPerson->hiv().isInfected() || pPerson->isPrep()))  // here we check age and woman again, if person is infected with HIV we drop out, if person is on Oral prep we also drop out
        {
            return true;
        }
    return false;
}

bool EventDVRDROP::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson, const State *pState) {
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double curTime = population.getTime();

    while (curTime==getNewInternalTimeDifference(pRndGen, pState)){  // dropoutevent is only possible at this time in population
        assert(m_DVRDROPprobDist);
        double dt = m_DVRDROPprobDist->pickNumber();

        if(dt > s_DVRDROPThreshold){
            return true;
        }
    }
    return false;
}

double EventDVRDROP::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    // we want an array of numbers where we can adjust the max limit in this array
    int randomNum = 0;
    randomNum = pRndGen->pickRandomInt(1, 10);
    // Multiply by 30.0 to get the time difference
    double dt = (randomNum * 30.0) / 365.0;
	return dt;
}


void EventDVRDROP::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);

    if (m_DVRDROP_enabled){
        if ((isEligibleForTreatment(t, pState, pPerson) && isWillingToStartTreatment(t, pRndGen, pPerson, pState)) || isHardDropOut(t, pState, pPerson))   // here we drop out do to normal conditions
        {
        WOMAN(pPerson)->setDVR(false);
        std::cout << "DVR DROP: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
        writeEventLogStart(true, "DVR_DROP", t, pPerson, 0);
        }
    }
}

ProbabilityDistribution *EventDVRDROP::m_DVRDROPprobDist = 0;


void EventDVRDROP::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Process DRV probability distribution
    if (m_DVRDROPprobDist) {
        delete m_DVRDROPprobDist;
        m_DVRDROPprobDist = 0;
    }
    m_DVRDROPprobDist = getDistributionFromConfig(config, pRndGen, "EventDVRDROP.m_DVRprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventDVRDROP.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventDVRDROP.threshold", s_DVRDROPThreshold)) ||
        !(r = config.getKeyValue("EventDVRDROP.schedulemin", s_DVRDROPScheduleMin)) ||
        !(r = config.getKeyValue("EventDVRDROP.schedulemax", s_DVRDROPScheduleMax))){
        abortWithMessage(r.getErrorString());
    }
    m_DVRDROP_enabled = (enabledStr == "true");

}

void EventDVRDROP::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventDVRDROP.enabled", m_DVRDROP_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventDVRDROP.threshold", s_DVRDROPThreshold)) ||
        !(r = config.addKey("EventDVRDROP.schedulemin", s_DVRDROPScheduleMin)) ||
        !(r = config.addKey("EventDVRDROP.schedulemax", s_DVRDROPScheduleMax))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_DVRDROPprobDist, config, "EventDVRDROP.m_DVRprobDist");
}

ConfigFunctions DVRDROPConfigFunctions(EventDVRDROP::processConfig, EventDVRDROP::obtainConfig, "EventDVRDROP");

JSONConfig DVRDROPJSONConfig(R"JSON(
    "EventDVRDROP": { 
        "depends": null,
        "params": [
            ["EventDVRDROP.enabled", "true", [ "true", "false"] ],
            ["EventDVRDROP.threshold", 0.5],
            ["EventDVRDROP.schedulemin", 1],
            ["EventDVRDROP.schedulemax", 5],
            ["EventDVRDROP.m_DVRDROPprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    }
)JSON");
