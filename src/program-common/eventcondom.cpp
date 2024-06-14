#include "eventcondom.h"
#include "gslrandomnumbergenerator.h"
#include "configdistributionhelper.h"
#include "util.h"
#include "configsettings.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configsettingslog.h"
#include <iostream>
#include <cstdlib> // for rand() function
#include <chrono>

using namespace std;

bool EventCondom::m_condom_enabled = false; // line here exists only for declartion, does not set default to false, that is set in cofig JSON at the bottom
double EventCondom::s_condomThreshold = 0.5; // Initialize with the default threshold value

EventCondom::EventCondom(Person *pPerson) : SimpactEvent(pPerson)
{
	assert(!pPerson->isCondomUsing());
}

EventCondom::~EventCondom()
{
}

string EventCondom::getDescription(double tNow) const
{
    Person *pPerson = getPerson(0);
	return strprintf("Condom Programming event for %s", getPerson(0)->getName().c_str());
}

void EventCondom::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
}

bool EventCondom::isEligibleForTreatment(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    
    Person *pPerson = getPerson(0);
    double curTime = population.getTime();
    double age = pPerson->getAgeAt(curTime); 
    // cout << "Checking eligibility for person " << pMan->getName() << " with age: " << age << endl;
    
    if (!pPerson->isCondomUsing() && age >= 15.0) {
        // cout << "Person " << pPerson->getName() << " Condom eligible with age: " << age << endl;
        return true;  // eligible for condom programming
    }
    return false; // not eligible for condom programming
}

bool EventCondom::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_condomprobDist);
	double dt = m_condomprobDist->pickNumber();
    if (dt > s_condomThreshold)
        return true;
    return false;
}

double EventCondom::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState, double t)
{
        double dt = t;
        // assert(m_condomscheduleDist);
        // if not in initialization phase
        // if (!initializationPhase)
        //     double dt = 0; // m_condomscheduleDist->pickNumber();
        return dt;
}

void EventCondom::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Person *pPerson = getPerson(0);
    double curTime = population.getTime();
    double age = pPerson->getAgeAt(curTime);
    assert(interventionTime == t);

    if (m_condom_enabled) {
        if (isEligibleForTreatment(t, pState) && isWillingToStartTreatment(t, pRndGen)) {
            assert(!pPerson->isCondomUsing());
            pPerson->setCondomUse(true);
            writeEventLogStart(true, "(Condom_Programming)", t, pPerson, 0);
        } 
    } 
}

ProbabilityDistribution *EventCondom::m_condomprobDist = 0;
ProbabilityDistribution *EventCondom::m_condomscheduleDist = 0;

void EventCondom::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Process Condom probability distribution
    if (m_condomprobDist) {
        delete m_condomprobDist;
        m_condomprobDist = 0;
    }
    m_condomprobDist = getDistributionFromConfig(config, pRndGen, "EventCondom.m_condomprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventCondom.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventCondom.threshold", s_condomThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_condom_enabled = (enabledStr == "true");
}

void EventCondom::obtainConfig(ConfigWriter &config) {
    bool_t r;

    if (!(r = config.addKey("EventCondom.enabled", m_condom_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventCondom.threshold", s_condomThreshold))) {
        abortWithMessage(r.getErrorString());
    }

    // Add the condom probability distribution to the config
    addDistributionToConfig(m_condomprobDist, config, "EventCondom.m_condomprobDist");
}

ConfigFunctions CondomConfigFunctions(EventCondom::processConfig, EventCondom::obtainConfig, "EventCondom");

JSONConfig CondomJSONConfig(R"JSON(
    "EventCondom": { 
        "depends": null,
        "params": [
            ["EventCondom.enabled", "true", [ "true", "false"] ],
            ["EventCondom.threshold", 0.5],
            ["EventCondom.m_condomprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept Condom treatment",
            "and to enable or disable the Condom event."
        ]
    }
)JSON");