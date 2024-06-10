#include "eventprep.h"
#include "gslrandomnumbergenerator.h"
#include "configdistributionhelper.h"
#include "util.h"
#include "configsettings.h"
#include "eventdropout.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configsettingslog.h"
#include <iostream>
#include <cstdlib> // for rand() function
#include <chrono>

using namespace std;

bool EventPrep::m_prep_enabled = false;

EventPrep::EventPrep(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson)
{
    m_scheduleImmediately = scheduleImmediately;
    assert(!pPerson->hiv().isDiagnosed());
    assert(pPerson->isSexuallyActive());
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

bool EventPrep::isEligibleForTreatment(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);

    Person *pPerson = getPerson(0);
    assert(!pPerson->hiv().isDiagnosed());
    assert(pPerson->isSexuallyActive());
    
    if (pPerson->getNumberOfRelationships() > 0 && !pPerson->isPrep()){  //we check that a person is in a relationship
        return true;
    }else{
        // set person property here back to 0.
        return false;
    }
}

bool EventPrep::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_prepprobDist);
	double dt = m_prepprobDist->pickNumber();
    if (dt > 0.5)  //threshold is 0.5
        return true;
    return false;
}

double EventPrep::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
    double dt = 0;
	if (m_scheduleImmediately)
	{
		double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
		return hour * pRndGen->pickRandomDouble();
	}else if (m_prepscheduleDist)
    {
        dt = m_prepscheduleDist->pickNumber();
	    return dt;
    }else{
        dt = 0.5;
        return dt;
    }
}


void EventPrep::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);
    double curTime = population.getTime();
    double age = pPerson->getAgeAt(curTime);
    assert(interventionTime == t); // make sure we're at the correct time

    if (m_prep_enabled) {
        if (isEligibleForTreatment(t, pState) && isWillingToStartTreatment(t, pRndGen)) {
            std::cout << "Before PREP status: " << pPerson->isPrep() << " for: " << pPerson->getName() << " Age: " << age << std::endl;
            writeEventLogStart(true, "Prep_treatment", t, pPerson, 0);
            pPerson->setPrep(true);
            std::cout << "After PREP status: " << pPerson->isPrep() << " for: " << pPerson->getName() << " Age: " << age << std::endl;
            // Dropout event becomes possible
		    // EventPrepDrop *pEvtPrepDrop = new EventPrepDrop(pPerson, t);
		    // population.onNewEvent(pEvtPrepDrop);
        } 
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
    if (!(r = config.getKeyValue("EventPrep.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false")) {
        abortWithMessage(r.getErrorString());
    }
    m_prep_enabled = (enabledStr == "true");

}

void EventPrep::obtainConfig(ConfigWriter &config) {
    bool_t r;

    // Add the VMMC enabled parameter
    if (!(r = config.addKey("EventPrep.enabled", m_prep_enabled ? "true" : "false"))) {
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
