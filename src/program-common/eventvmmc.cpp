#include "eventvmmc.h"
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

double EventVMMC::s_vmmcThreshold = 0.5;
bool EventVMMC::m_VMMC_enabled = false; // line here exists only for declartion, does not set default to false, that is set in cofig JSON at the bottom

EventVMMC::EventVMMC(Person *pMan) : SimpactEvent(pMan)
{
	assert(pMan->isMan());
}

EventVMMC::~EventVMMC()
{
}

string EventVMMC::getDescription(double tNow) const
{
    Person *pMan = MAN(getPerson(0));
    assert(pMan->isMan());
	return strprintf("VMMC event for %s", getPerson(0)->getName().c_str());
}

void EventVMMC::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pMan = MAN(getPerson(0));
    assert(pMan->isMan());
}

bool EventVMMC::isEligibleForTreatment(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    
    Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());   // we assert that a person is from the male class
    double curTime = population.getTime();
    double age = pMan->getAgeAt(curTime); 
    // cout << "Checking eligibility for person " << pMan->getName() << " with age: " << age << endl;
    
    if (pMan->isMan() && !pMan->isVmmc() && age >= 15.0 && age <= 49.0) {
        // cout << "Person " << pMan->getName() << " eligible with age: " << age << endl;
        return true;  // eligible for treatment
    }
    return false; // not eligible for treatment
}

bool EventVMMC::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_vmmcprobDist);
	double dt = m_vmmcprobDist->pickNumber();
    if (dt > s_vmmcThreshold)  //threshold is 0.5
        return true;
    return false;
}

double EventVMMC::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(m_vmmcscheduleDist);

	double dt = m_vmmcscheduleDist->pickNumber();

	return dt;
}

void EventVMMC::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());
    double curTime = population.getTime();
    double age = pMan->getAgeAt(curTime);
    assert(interventionTime == t); // make sure we're at the correct time

    if (m_VMMC_enabled) {
        if (isEligibleForTreatment(t, pState) && isWillingToStartTreatment(t, pRndGen) && pMan->isMan()) {
            assert(!pMan->isVmmc());
            // std::cout << "Circumcising Person: " << pMan->getName() << " Age: " << age << std::endl; // Debugging statement
            pMan->setVmmc(true);
            writeEventLogStart(true, "(VMMC_treatment)", t, pMan, 0);
        } 
    } 
}

ProbabilityDistribution *EventVMMC::m_vmmcprobDist = 0;
ProbabilityDistribution *EventVMMC::m_vmmcscheduleDist = 0;

void EventVMMC::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;
    
    // Process VMMC schedule distribution
    if (m_vmmcscheduleDist) {
        delete m_vmmcscheduleDist;
        m_vmmcscheduleDist = 0;
    }
    m_vmmcscheduleDist = getDistributionFromConfig(config, pRndGen, "EventVMMC.m_vmmcscheduleDist");

    // Process VMMC probability distribution
    if (m_vmmcprobDist) {
        delete m_vmmcprobDist;
        m_vmmcprobDist = 0;
    }
    m_vmmcprobDist = getDistributionFromConfig(config, pRndGen, "EventVMMC.m_vmmcprobDist");

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventVMMC.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false") ||
        !(r = config.getKeyValue("EventVMMC.threshold", s_vmmcThreshold))){
        abortWithMessage(r.getErrorString());
    }
    m_VMMC_enabled = (enabledStr == "true");
    
    // Debugging statement
    // std::cout << "VMMC enabled: " << m_VMMC_enabled << std::endl;
}

void EventVMMC::obtainConfig(ConfigWriter &config) {
    bool_t r;

    // Add the VMMC enabled parameter
    if (!(r = config.addKey("EventVMMC.enabled", m_VMMC_enabled ? "true" : "false")) ||
        !(r = config.addKey("EventVMMC.threshold", s_vmmcThreshold))) {
        abortWithMessage(r.getErrorString());
    }

    // Add the VMMC schedule distribution to the config
    addDistributionToConfig(m_vmmcscheduleDist, config, "EventVMMC.m_vmmcscheduleDist");

    // Add the VMMC probability distribution to the config
    addDistributionToConfig(m_vmmcprobDist, config, "EventVMMC.m_vmmcprobDist");
}

ConfigFunctions VMMCConfigFunctions(EventVMMC::processConfig, EventVMMC::obtainConfig, "EventVMMC");

JSONConfig VMMCJSONConfig(R"JSON(
    "EventVMMC": { 
        "depends": null,
        "params": [
            ["EventVMMC.enabled", "true", [ "true", "false"] ],
            ["EventVMMC.threshold", 0.5],
            ["EventVMMC.m_vmmcprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept VMMC treatment",
            "and to enable or disable the VMMC event."
        ]
    },
    "EventVMMC_schedule_dist": { 
        "depends": null,
        "params": [  
            [ "EventVMMC.m_vmmcscheduleDist.dist", "distTypes", ["fixed", [ ["value", 0.246575 ] ] ] ] 
        ],
        "info": [ 
            "This parameter is used to specify the VMMC scheduling duration. The default is fixed."
        ]
    }
)JSON");