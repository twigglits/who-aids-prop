#include "eventvmmcseed.h"
#include "eventaidsmortality.h"
#include "eventhivtransmission.h"
#include "eventchronicstage.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>
#include <cmath>

using namespace std;

EventVMMCSeed::EventVMMCSeed()
{
}

EventVMMCSeed::~EventVMMCSeed()
{
}

double EventVMMCSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	return EventSeedBase::getNewInternalTimeDifference(s_settings, pRndGen, pState);
}

string EventVMMCSeed::getDescription(double tNow) const
{
	return "HIV seeding";
}

void EventVMMCSeed::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	writeEventLogStart(true, "HIV seeding", tNow, 0, 0);
}

void EventVMMCSeed::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	EventSeedBase::fire(s_settings, t, pState, EventHIVTransmission::infectPerson);
}

SeedEventSettings EventVMMCSeed::s_settings;

void EventVMMCSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventSeedBase::processConfig(s_settings, config, pRndGen, "hivseed");
}

void EventVMMCSeed::obtainConfig(ConfigWriter &config)
{
	EventSeedBase::obtainConfig(s_settings, config, "hivseed");
}

ConfigFunctions hivseedingConfigFunctions(EventVMMCSeed::processConfig, EventVMMCSeed::obtainConfig, "EventHIVSeed");

// The 0 is the default seed time; HIV seeding by default is at the start of the simulation
JSONConfig hivseedingJSONConfig(EventSeedBase::getJSONConfigText("EventHIVSeeding", "hivseed", "HIV", 0));
