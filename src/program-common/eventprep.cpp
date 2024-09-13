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
double EventPrep::s_prepAGYWThreshold = 0.5;

EventPrep::EventPrep(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventPrep::~EventPrep()
{
}

// rest of our template functions
string EventPrep::getDescription(double tNow) const
{
	return strprintf("Prep event for %s", getPerson(0)->getName().c_str());
}

void EventPrep::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventPrep::isEligibleForTreatmentP1(double t, const State *pState, Person *pPerson)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    // Person *pPerson1 = getPerson(0);
    // Person *pPerson2 = getPerson(1);

    if (!pPerson->hiv().isInfected() && !pPerson->isPrep()){  //&& pPerson2->hiv().isInfected()  we check that a person is in a relationship
        // std::cout << "P1 eligible: " << pPerson2->getName() << std::endl;
        return true;
    }else{
        // std::cout << "P1 NOT ELIGIBLE: " << pPerson2->getName() << std::endl;
        return false;
    }
}

bool EventPrep::isWillingToStartTreatmentP1(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson) {
    // Person *pPerson = getPerson(0);
    assert(m_prepprobDist);
    double dt = m_prepprobDist->pickNumber();
    double threshold=0;  
    if (pPerson->isWoman() && WOMAN(pPerson)->isAGYW()){
        threshold = s_prepAGYWThreshold;
    }
    else
    {
        threshold = s_prepThreshold;
    }
    if(dt > threshold){
        return true;
    }
    return false;
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
    Person *pPerson = getPerson(0);

    if (isEligibleForTreatmentP1(t, pState, pPerson) && isWillingToStartTreatmentP1(t, pRndGen, pPerson)) 
    {
    pPerson->setPrep(true);
    std::cout << "P FIRE: " << pPerson->getName() << "Gender"<< pPerson->getGender() << std::endl;
    writeEventLogStart(true, "Prep_treatment_P1", t, pPerson, 0);
    
	EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson, t);  // needs to be smaller percentage than those that took up prep
	population.onNewEvent(pEvtPrepDrop);
    }
}

ProbabilityDistribution *EventPrep::m_prepprobDist = 0;
ProbabilityDistribution *EventPrep::m_prepscheduleDist = 0;
PieceWiseLinearFunction *EventPrep::s_pRecheckInterval = 0;

void EventPrep::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;
    
    // Process VMMC schedule distribution
    if (m_prepscheduleDist) {
        delete m_prepscheduleDist;
        m_prepscheduleDist = 0;
    }
    m_prepscheduleDist = getDistributionFromConfig(config, pRndGen, "EventPrep.m_prepscheduleDist");

    // Process VMMC probability distribution
    if (m_prepprobDist) {
        delete m_prepprobDist;
        m_prepprobDist = 0;
    }
    m_prepprobDist = getDistributionFromConfig(config, pRndGen, "EventPrep.m_prepprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventPrep.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventPrep.AGYWthreshold", s_prepAGYWThreshold)) || 
        !(r = config.getKeyValue("EventPrep.threshold", s_prepThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_prep_enabled = (enabledStr == "true");

}

void EventPrep::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventPrep.enabled", m_prep_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventPrep.AGYWthreshold", s_prepAGYWThreshold)) ||
        !(r = config.addKey("EventPrep.threshold", s_prepThreshold))){
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
            ["EventPrep.AGYWthreshold", 0.5],
            ["EventPrep.threshold", 0.5],
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
