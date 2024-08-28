#include "eventagyw.h"
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

bool EventAGYW::m_AGYW_enabled = false; 

EventAGYW::EventAGYW(Person *pWoman) : SimpactEvent(pWoman)
{
	assert(pWoman->isWoman());
}

EventAGYW::~EventAGYW()
{
}

string EventAGYW::getDescription(double tNow) const
{
    Person *pWoman = WOMAN(getPerson(0));
    assert(pWoman->isWoman());
	return strprintf("AGYW event for %s", getPerson(0)->getName().c_str());
}

void EventAGYW::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pWoman = WOMAN(getPerson(0));
    assert(pWoman->isWoman());
}

bool EventAGYW::isEligibleForTreatment(double t, const State *pState)
{
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    
    Woman *pWoman = WOMAN(getPerson(0));
    assert(pWoman->isWoman());// line here exists only for declartion, does not set default to false, that is set in cofig JSON at the bottom
    double curTime = population.getTime();
    double age = pWoman->getAgeAt(curTime); 
    // cout << "Checking eligibility for person " << pWoman->getName() << " with age: " << age << endl;
    
    if (pWoman->isWoman() && age >= 15.0 && age < 25.0) { 
        // cout << "Person: " << pWoman->getName() << " eligible with Age: " << age << " and Gender: " << pWoman->getGender() << endl;
        return true;
    }    
    return false;
}

double EventAGYW::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(m_agywscheduleDist);
	double dt = m_agywscheduleDist->pickNumber();
	return dt;
}

void EventAGYW::fire(Algorithm *pAlgorithm, State *pState, double t) {
    SimpactPopulation &population = SIMPACTPOPULATION(pState);
    double interventionTime;
    ConfigSettings interventionConfig;

    GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
    Woman *pWoman = WOMAN(getPerson(0));
    assert(pWoman->isWoman());
    double curTime = population.getTime();
    double age = pWoman->getAgeAt(curTime);
    assert(interventionTime == t);

    if (m_AGYW_enabled) {
        if (isEligibleForTreatment(t, pState) && pWoman->isWoman()) {
            assert(!pWoman->isAGYW());
            pWoman->setAGYW(true);
            std::cout << "Adding Woman to AGYW selection: " << pWoman->getName() << " Age: " << age << "AGYW status: " << pWoman->isAGYW() << std::endl;
            writeEventLogStart(true, "(AGYW_selection)", t, pWoman, 0);
        }else if (!isEligibleForTreatment(t, pState) && pWoman->isWoman()){
            assert(pWoman->isAGYW());
            pWoman->setAGYW(false);
            std::cout << "Removing Woman from AGYW selection: " << pWoman->getName() << " Age: " << age << "AGYW status: " << pWoman->isAGYW() << std::endl;
            writeEventLogStart(false, "(AGYW_removal)", t, pWoman, 0);
        }
    } 
}

ProbabilityDistribution *EventAGYW::m_agywscheduleDist = 0;

void EventAGYW::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventAGYW.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false")){
        abortWithMessage(r.getErrorString());
    }
    m_AGYW_enabled = (enabledStr == "true");
}

void EventAGYW::obtainConfig(ConfigWriter &config) {
    bool_t r;

    // Add the AGYW enabled parameter
    if (!(r = config.addKey("EventAGYW.enabled", m_AGYW_enabled ? "true" : "false"))) {
        abortWithMessage(r.getErrorString());
    }
}

ConfigFunctions AGYWConfigFunctions(EventAGYW::processConfig, EventAGYW::obtainConfig, "EventAGYW");

JSONConfig AGYWJSONConfig(R"JSON(
    "EventAGYW": { 
        "depends": null,
        "params": [
            ["EventAGYW.enabled", "true", [ "true", "false"] ]
        ],
        "info": [ 
            "This parameter is used to set the distribution of subject willing to accept AGYW treatment",
            "and to enable or disable the AGYW event."
        ]
    }
)JSON");