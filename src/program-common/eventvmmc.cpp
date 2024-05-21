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

bool EventVMMC::isEligibleForTreatment(double t, const State *pState)
{
    // Person must be male and 15 years or older to be eligible for treatment.
    const SimpactPopulation &population = SIMPACTPOPULATION(pState);
    
    Man *pMan = MAN(getPerson(0));
    assert(pMan->isMan());   // we assert that a person is from the male class
    double curTime = population.getTime();
    double age = pMan->getAgeAt(curTime); 
    if (pMan->isMan() && !pMan->isVmmc() && age <= 10.0)   //if person is male & not yet circumsized & age 15 or older
	    return true;  // eligible for treatment
    else 
        return false; // not eligible for treatment
}

bool EventVMMC::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
    assert(m_vmmcprobDist);
	double dt = m_vmmcprobDist->pickNumber();
    if (dt > 0.5)  //threshold is 0.5
        return true;
    return false;
}

double EventVMMC::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(m_VMMCscheduleDist);

	double dt = m_VMMCscheduleDist->pickNumber();

	return dt;
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
	
	if (isEligibleForTreatment(t, pState) && isWillingToStartTreatment(t, pRndGen) && pMan->isMan()) // && m_VMMC_enabled==true)
        assert(!pMan->isVmmc());
        
        pMan->setVmmc(true);
		writeEventLogStart(true, "(VMMC_treatment)", t, pMan, 0);
}

ProbabilityDistribution *EventVMMC::m_vmmcprobDist = 0;
ProbabilityDistribution *EventVMMC::m_VMMCscheduleDist = 0;

void EventVMMC::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
    bool_t r;
    
    if (m_VMMCscheduleDist)
	{
		delete m_VMMCscheduleDist;
		m_VMMCscheduleDist = 0;
	}

	m_VMMCscheduleDist = getDistributionFromConfig(config, pRndGen, "EventVMMC.m_VMMCscheduleDist");
	
	if (m_vmmcprobDist)
	
		delete m_vmmcprobDist;
		m_vmmcprobDist = 0;

	m_vmmcprobDist = getDistributionFromConfig(config, pRndGen, "EventVMMC.m_vmmcprobDist");
    
}

void EventVMMC::obtainConfig(ConfigWriter &config)
{
	bool_t r;

    addDistributionToConfig(m_VMMCscheduleDist, config, "EventVMMC.m_VMMCscheduleDist");
	addDistributionToConfig(m_vmmcprobDist, config, "EventVMMC.m_vmmcprobDist");
}

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
	},
    "EventVMMC_schedule_dist": { 
            "depends": null,
            "params": [ [ 
                "EventVMMC.m_VMMCscheduleDist.dist", "distTypes", ["fixed", [ ["value", 0.246575 ] ] ] 
                ] 
            ],
            "info": [ 
                "This parameter is used to specify the VMMC scheduling duration. The default",
                "is the fixed value of 90/365"
            ]
    
    })JSON");