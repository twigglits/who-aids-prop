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

EventAGYW::EventAGYW(Person *pWoman) : SimpactEvent(pWoman)
{
}

EventAGYW::~EventAGYW()
{
}

double EventAGYW::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);
    assert(pPerson->isWoman());
    assert(m_AGYW_age > 0);
    
    // double age = pPerson->getDateOfBirth() + m_AGYW_age;
	// double dt = population.getTime() - age;

    double dt = 0.005;
    return dt;
}

std::string EventAGYW::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	return strprintf("AGYW desc %s", pPerson->getName().c_str());
}

void EventAGYW::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pWoman = getPerson(0);
    writeEventLogStart(true, "AGYW writelog", tNow, pWoman, 0);
}

void EventAGYW::fire(Algorithm *pAlgorithm, State *pState, double t) 
{
    // SimpactPopulation &population = SIMPACTPOPULATION(pState);
    Person *pWoman = getPerson(0);
	// assert(getNumberOfPersons() == 1);
    // assert(pWoman->isWoman());
    // double age = pWoman->getDateOfBirth() + m_AGYW_age;
	// double ref_age = population.getTime() - age;

    const SimpactPopulation &population_const = SIMPACTPOPULATION(pState);
	double curTime = population_const.getTime();
    double age = pWoman->getAgeAt(curTime); 

    if (age >= 15.0 && age < 25.0) {
		WOMAN(pWoman)->setAGYW(true);
        std::cout << "Setting AGYW: " << pWoman->getName() << "AGYW status: " << WOMAN(pWoman)->isAGYW() << std::endl;
        writeEventLogStart(true, "AGYW of", t, pWoman, 0);
    }else
    {
        WOMAN(pWoman)->setAGYW(false);
        std::cout << "Unsetting AGYW: " << pWoman->getName() << "AGYW status: " << WOMAN(pWoman)->isAGYW() << std::endl;
        writeEventLogStart(true, "(AGYW_Removal)", t, pWoman, 0);
    }
}     

bool EventAGYW::m_AGYW_enabled = false; 
double EventAGYW::m_AGYW_age = -1;

void EventAGYW::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen) {
    bool_t r;

    // Read the boolean parameter from the config
    std::string enabledStr;
    if (!(r = config.getKeyValue("EventAGYW.agywage", m_AGYW_age, 0, 100)) ||
        !(r = config.getKeyValue("EventAGYW.enabled", enabledStr)) || (enabledStr != "true" && enabledStr != "false"))
        {
        abortWithMessage(r.getErrorString());
    }
    m_AGYW_enabled = (enabledStr == "true");
}

void EventAGYW::obtainConfig(ConfigWriter &config) {
    bool_t r;

    // Add the AGYW enabled parameter
    if (!(r = config.addKey("EventAGYW.agywage", m_AGYW_age)) ||
        !(r = config.addKey("EventAGYW.enabled", m_AGYW_enabled ? "true" : "false"))){
        abortWithMessage(r.getErrorString());
    }
}

ConfigFunctions AGYWConfigFunctions(EventAGYW::processConfig, EventAGYW::obtainConfig, "EventAGYW");

JSONConfig AGYWJSONConfig(R"JSON(
    "EventAGYW": { 
        "depends": null,
        "params": [
            ["EventAGYW.agywage", 25 ],
            ["EventAGYW.enabled", "true", [ "true", "false"] ]
        ],
        "info": [ 
            "agywage represents the age at which we set the AGYW property to False",
            "and to enable or disable the AGYW event."
        ]
    }
)JSON");