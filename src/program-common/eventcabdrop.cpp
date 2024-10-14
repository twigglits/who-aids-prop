#include "eventcabdrop.h"
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

bool EventCABDROP::m_CABDrop_enabled = true;
double EventCABDROP::s_CABDropThreshold = 0.5;

EventCABDROP::EventCABDROP(Person *pPerson) : SimpactEvent(pPerson)
{
    assert(pPerson->isCAB());
    if (!pPerson->isCAB()) {
        // Handle the error: avoid creating an invalid event
        // Option 1: Throw an exception
        throw std::invalid_argument("Attempted to create EventCABDROP for person not on CAB.");
        
        // Option 2: Log an error and set CAB to false (if throwing exceptions is not feasible)
        std::cerr << "Error: EventCABDROP created for person not on CAB: " << pPerson->getName() << std::endl;
        // Optionally, you can decide not to proceed further
    }
}

EventCABDROP::~EventCABDROP()
{
}

// rest of our template functions
string EventCABDROP::getDescription(double tNow) const
{
	return strprintf("CABDROP event for %s", getPerson(0)->getName().c_str());
}

void EventCABDROP::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventCABDROP::isEligibleForTreatment(double t, const State *pState, Person *pPerson)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    if (pPerson->isCAB()){
        return true;
    }
    return false;
}

bool EventCABDROP::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson) {
    assert(m_CABDROPprobDist);
    double dt = m_CABDROPprobDist->pickNumber();

    if(dt > s_CABDropThreshold){
        return true;
    }
    return false;
}

bool EventCABDROP::isHardDropOut(double t, const State *pState, Person *pPerson){
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    if(pPerson->isWoman() && WOMAN(pPerson)->isDVR() && pPerson->isCAB()){
        return true;
    }
    else if(pPerson->isCAB() && (pPerson->hiv().isInfected() || pPerson->isPrep() || pPerson->getNumberOfRelationships()==0))
    {
        return true;
    }
    return false;
}

double EventCABDROP::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    // double dt = 0.1;
    // return dt;
    Person *pPerson = getPerson(0);
    if (pPerson->isCAB()) {
        return 1.0; // Reschedule after 1.0 time units if still on CAB
    }
    return std::numeric_limits<double>::infinity(); // Do not reschedule otherwise
}

// void EventCABDROP::fire(Algorithm *pAlgorithm, State *pState, double t) {
//     SimpactPopulation &population = SIMPACTPOPULATION(pState);
//     double interventionTime;
//     ConfigSettings interventionConfig;

//     GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
//     Person *pPerson = getPerson(0);

//     if (m_CABDrop_enabled){
//         if (pPerson->isCAB() && isEligibleForTreatment(t, pState, pPerson) && isHardDropOut(t, pState, pPerson)){
//             pPerson->setCAB(false);
//             std::cout << "CAB_DROP_HARD FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
//             writeEventLogStart(true, "CAB_DROP", t, pPerson, 0);
//         }else if(pPerson->isCAB() && isEligibleForTreatment(t, pState, pPerson) && isWillingToStartTreatment(t, pRndGen, pPerson))
//         {
//             pPerson->setCAB(false);
//             std::cout << "CAB_DROP_SOFT FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
//             writeEventLogStart(true, "CAB_DROP", t, pPerson, 0);
//         }
//     }
// }

void EventCABDROP::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);

    std::cout << "Firing EventCABDROP for: " << pPerson->getName() << " at time: " << t << std::endl;

    if (m_CABDrop_enabled && pPerson->isCAB()) {
        if (isHardDropOut(t, pState, pPerson)) {
            pPerson->setCAB(false);
            pPerson->setCABDropEventScheduled(false); // Unset the flag
            std::cout << "CAB_DROP_HARD FIRE: " << pPerson->getName() 
                      << " Gender: " << pPerson->getGender() << std::endl;
            writeEventLogStart(true, "CAB_DROP", t, pPerson, 0);
        }
        else if (isWillingToStartTreatment(t, pRndGen, pPerson)) {
            pPerson->setCAB(false);
            pPerson->setCABDropEventScheduled(false); // Unset the flag
            std::cout << "CAB_DROP_SOFT FIRE: " << pPerson->getName() 
                      << " Gender: " << pPerson->getGender() << std::endl;
            writeEventLogStart(true, "CAB_DROP", t, pPerson, 0);
        }
    }
}

ProbabilityDistribution *EventCABDROP::m_CABDROPprobDist = 0;


void EventCABDROP::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Process DRV probability distribution
    if (m_CABDROPprobDist) {
        delete m_CABDROPprobDist;
        m_CABDROPprobDist = 0;
    }
    m_CABDROPprobDist = getDistributionFromConfig(config, pRndGen, "EventCABDROP.m_CABDROPprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventCABDROP.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventCABDROP.threshold", s_CABDropThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_CABDrop_enabled = (enabledStr == "true");

}

void EventCABDROP::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventCABDROP.enabled", m_CABDrop_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventCABDROP.threshold", s_CABDropThreshold))){
        abortWithMessage(r.getErrorString());
    }

    addDistributionToConfig(m_CABDROPprobDist, config, "EventCABDROP.m_CABDROPprobDist");
}

ConfigFunctions CABDROPConfigFunctions(EventCABDROP::processConfig, EventCABDROP::obtainConfig, "EventCABDROP");

JSONConfig CABDROPJSONConfig(R"JSON(
    "eventCABDROP": { 
        "depends": null,
        "params": [
            ["EventCABDROP.enabled", "true", [ "true", "false"] ],
            ["EventCABDROP.threshold", 0.5],
            ["EventCABDROP.m_CABDROPprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept CAB treatment",
            "and to enable or disable the CAB event."
        ]
    }
)JSON");