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

EventPrep::EventPrep(Person *pPerson1, Person *pPerson2, bool scheduleImmediately) : SimpactEvent(pPerson1, pPerson2)
{
    // m_scheduleImmediately = scheduleImmediately;
    // assert(pPerson1->hasRelationshipWith(pPerson2));
    // assert(pPerson2->hasRelationshipWith(pPerson1));
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

bool EventPrep::isEligibleForTreatmentP1(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    Person *pPerson1 = getPerson(0);
    Person *pPerson2 = getPerson(1);

    if (!pPerson1->hiv().isInfected() && !pPerson1->isPrep() && pPerson2->hiv().isInfected()){  //we check that a person is in a relationship
        // std::cout << "P1 eligible: " << pPerson2->getName() << std::endl;
        return true;
    }else{
        // std::cout << "P1 NOT ELIGIBLE: " << pPerson2->getName() << std::endl;
        return false;
    }
}

bool EventPrep::isEligibleForTreatmentP2(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    Person *pPerson1 = getPerson(0);
    Person *pPerson2 = getPerson(1);

    if (!pPerson2->hiv().isInfected() && !pPerson2->isPrep() && pPerson1->hiv().isInfected()){  //we check that a person is in a relationship
        // std::cout << "P2 eligible: " << pPerson2->getName() << std::endl;
        return true;
    }else{
        // std::cout << "P2 NOT ELIGIBLE: " << pPerson2->getName() << std::endl;
        return false;
    }
}

bool EventPrep::isWillingToStartTreatmentP1(double t, GslRandomNumberGenerator *pRndGen) {
    //Person *pPerson1 = getPerson(0);
    assert(m_prepprobDist);
    double dt = m_prepprobDist->pickNumber();
    if (dt > s_prepThreshold )
        return true;
    return false;
}

bool EventPrep::isWillingToStartTreatmentP2(double t, GslRandomNumberGenerator *pRndGen) {
    //Person *pPerson2 = getPerson(1);
    assert(m_prepprobDist);
    double dt = m_prepprobDist->pickNumber();
    if (dt > s_prepThreshold )
        return true;
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
    Person *pPerson1 = getPerson(0);
    Person *pPerson2 = getPerson(1);
    double curTime = population.getTime();
    double age1 = pPerson1->getAgeAt(curTime);
    double age2 = pPerson2->getAgeAt(curTime);
    assert(interventionTime == t); // make sure we're at the correct time

    // if (m_prep_enabled) {

    if (isEligibleForTreatmentP1(t, pState) && isWillingToStartTreatmentP1(t, pRndGen)) 
    {
    pPerson1->setPrep(true);
    writeEventLogStart(true, "Prep_treatment_P1", t, pPerson1, 0);
    // std::cout << "After PREP_P1 status: " << pPerson1->isPrep() << " for P1: " << pPerson1->getName() << " Age: " << age1 << std::endl;
    // Dropout event becomes possible
	EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson1, t);  // needs to be smaller percentage than those that took up prep
	population.onNewEvent(pEvtPrepDrop);
    }

    if (isEligibleForTreatmentP2(t, pState) && isWillingToStartTreatmentP2(t, pRndGen)) 
    {
        pPerson2->setPrep(true);
        writeEventLogStart(true, "Prep_treatment_P2", t, pPerson2, 0);
        // std::cout << "After PREP_P2 status: " << pPerson2->isPrep() << " for: " << pPerson2->getName() << " Age: " << age2 << std::endl;
        // Dropout event becomes possible
		EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson2, t);  // needs to be smaller percentage than those that took up prep
		population.onNewEvent(pEvtPrepDrop);
    } 
    // } 
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
        !(r = config.getKeyValue("EventPrep.threshold", s_prepThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_prep_enabled = (enabledStr == "true");

}

void EventPrep::obtainConfig(ConfigWriter &config) {
    bool_t r;

    // Add the VMMC enabled parameter
    if (!(r = config.addKey("EventPrep.enabled", m_prep_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventPrep.threshold", s_prepThreshold))) {
        abortWithMessage(r.getErrorString());
    }

    // Add the VMMC schedule distribution to the config
    addDistributionToConfig(m_prepscheduleDist, config, "EventPrep.m_prepscheduleDist");

    // Add the VMMC probability distribution to the config
    addDistributionToConfig(m_prepprobDist, config, "EventPrep.m_prepprobDist");
}

ConfigFunctions PrepConfigFunctions(EventPrep::processConfig, EventPrep::obtainConfig, "EventPrep");

JSONConfig PrepJSONConfig(R"JSON(
    "EventPrep": { 
        "depends": null,
        "params": [
            ["EventPrep.enabled", "true", [ "true", "false"] ],
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
