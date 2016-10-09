#include "eventsyncpopstats.h"
#include "jsonconfig.h"
#include "configfunctions.h"

using namespace std;

EventSyncPopulationStatistics::EventSyncPopulationStatistics()
{
}

EventSyncPopulationStatistics::~EventSyncPopulationStatistics()
{
}

string EventSyncPopulationStatistics::getDescription(double tNow) const
{
	return "syncpopstats";
}

void EventSyncPopulationStatistics::writeLogs(double tNow) const
{
	writeEventLogStart(true, "syncpopstats", tNow, 0, 0);
}

void EventSyncPopulationStatistics::fire(State *pState, double t)
{
	// Currently only the population size
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	population.setLastKnownPopulationSize();

	double lastTime = 0;
	int lastSize = population.getLastKnownPopulationSize(lastTime);

	writeEventLogStart(false, "(populationsize)", t, 0, 0);
	LogEvent.print(",size,%d", lastSize);

	if (isEnabled())
	{
		EventSyncPopulationStatistics *pEvt = new EventSyncPopulationStatistics();
		population.onNewEvent(pEvt);
	}
}

double EventSyncPopulationStatistics::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(s_interval > 0);
	return s_interval;
}

double EventSyncPopulationStatistics::s_interval = -1.0;

void EventSyncPopulationStatistics::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (!config.getKeyValue("syncpopstats.interval", s_interval))
		abortWithMessage(config.getErrorString());
}

void EventSyncPopulationStatistics::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("syncpopstats.interval", s_interval))
		abortWithMessage(config.getErrorString());
}

ConfigFunctions eventSyncPopStatsConfigFunctions(EventSyncPopulationStatistics::processConfig,
		                                         EventSyncPopulationStatistics::obtainConfig,
												 "EventSyncPopulationStatistics");

JSONConfig eventSyncPopStatsJSONConfig(R"JSON(
		"EventSyncPopStats": {
			"depends": null,
			"params": [
				[ "syncpopstats.interval", "-1" ]
			],
			"info": [
				"TODO"
			]
		})JSON");
