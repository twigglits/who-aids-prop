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

EventVMMC::EventVMMC(Person *pMan) : SimpactEvent(pMan)
{
	assert(pMan->isMan());
    // assert(m_VMMC_enabled);
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

bool EventVMMC::isEligibleForTreatment(double t)
{
    // Person must be male and 15 years or older to be eligible for treatment.
    Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());   // we assert that a person is from the male class
    
    double age = pMan->getAgeAt(t);
    if (pMan->isMan() && !pMan->isVmmc() && age >= 15.0 && age <= 49.0)   //if person is male & not yet circumsized & age 15 or older
	    return true;  // eligible for treatment
    else 
        return false; // not eligible for treatment
}

ProbabilityDistribution *EventVMMC::m_vmmcprobDist = 0;
// bool EventVMMC::m_VMMC_enabled = false;

bool EventVMMC::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_vmmcprobDist);
	double dt = m_vmmcprobDist->pickNumber();
    if (dt > 0.5)  //threshold is 0.5
        return true;
    return false;
}

void EventVMMC::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	double interventionTime;
	ConfigSettings interventionConfig;
	
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());
    assert(interventionTime == t); // make sure we're at the correct time
	
	if (isEligibleForTreatment(t) && isWillingToStartTreatment(t, pRndGen) && pMan->isMan()) // && m_VMMC_enabled==true)
        pMan->setVmmc(true);
		writeEventLogStart(true, "(VMMC_treatment)", t, pMan, 0);
}

void EventVMMC::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	
	if (m_vmmcprobDist)
	
		delete m_vmmcprobDist;
		m_vmmcprobDist = 0;
	
	bool_t r;

	// if (!(r = config.getKeyValue("vmmc.enabled", EventVMMC::m_VMMC_enabled)))
		// abortWithMessage(r.getErrorString());

	m_vmmcprobDist = getDistributionFromConfig(config, pRndGen, "EventVMMC.m_vmmcprobDist");
}

void EventVMMC::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	addDistributionToConfig(m_vmmcprobDist, config, "EventVMMC.m_vmmcprobDist");

	// if(!(r = config.addKey("vmmc.enabled", EventVMMC::m_VMMC_enabled)))
	  // abortWithMessage(r.getErrorString());

}

	   // "EventVMMC": {
	   // "depends": null,
	   // "params": [ ["vmmc.enabled", "yes", ["yes", "no"] ] ],
	   // "info": [
	   // "When value is set to yes then there is some distribution of males that get circumsized",
	   // "Default is yes"
	   // ]
	   // },

ConfigFunctions VMMCConfigFunctions(EventVMMC::processConfig, EventVMMC::obtainConfig, "EventVMMC");

JSONConfig VMMCJSONConfig(R"JSON(

	"EventVMMC_dist": { 
            "depends": null,
            "params": [ 
                [ "EventVMMC.m_vmmcprobDist.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 1 ] ] ] ]
            ],
            "info": [ 
                "TODO"
            ]
	})JSON");