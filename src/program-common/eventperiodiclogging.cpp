#include "eventperiodiclogging.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include <iostream>

using namespace std;

EventPeriodicLogging::EventPeriodicLogging() // global event
{
}

EventPeriodicLogging::~EventPeriodicLogging()
{
}

double EventPeriodicLogging::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	assert(s_loggingInterval > 0);

	return s_loggingInterval;
}

string EventPeriodicLogging::getDescription(double tNow) const
{
	return "Periodic logging";
}

void EventPeriodicLogging::writeLogs(const Population &pop, double tNow) const
{
	writeEventLogStart(true, "periodiclogging", tNow, 0, 0);
}

void EventPeriodicLogging::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);

	// Count number of people currently in treatment
	Person **ppPeople = population.getAllPeople();
	int numPeople = population.getNumberOfPeople();

	int inTreatmentCount = 0;
	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];
		assert(pPerson);

		if (pPerson->isInfected() && pPerson->hasLoweredViralLoad())
			inTreatmentCount++;
	}

	s_logFile.print("%10.10f,%d,%d", t, numPeople, inTreatmentCount);

	// Schedule next logging event

	if (s_loggingInterval > 0) // make sure it hasn't been disabled (by an intervention event for example)
	{
		// We need to schedule the next one
		EventPeriodicLogging *pEvt = new EventPeriodicLogging();
		population.onNewEvent(pEvt);
	}
}

double EventPeriodicLogging::s_loggingInterval = -1;
string EventPeriodicLogging::s_logFileName;
LogFile EventPeriodicLogging::s_logFile;

void EventPeriodicLogging::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	string oldLogFileName = s_logFileName;

	if (!config.getKeyValue("periodiclogging.interval", s_loggingInterval) ||
	    !config.getKeyValue("periodiclogging.outfile.logperiodic", s_logFileName) )
		abortWithMessage(config.getErrorString());

	if (s_loggingInterval > 0)
	{
		if (oldLogFileName != s_logFileName) // other file was specified, or none at all
		{
			s_logFile.close();
			if (s_logFileName.length() > 0) // try to open a file
			{
				if (!s_logFile.open(s_logFileName))
					abortWithMessage(s_logFile.getErrorString());

				s_logFile.print("Time,PopSize,InTreatment");
			}
		}
	}
}

void EventPeriodicLogging::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("periodiclogging.interval", s_loggingInterval) ||
	    !config.addKey("periodiclogging.outfile.logperiodic", s_logFileName) )
	    	abortWithMessage(config.getErrorString());
}

ConfigFunctions periodicLoggingConfigFunctions(EventPeriodicLogging::processConfig, EventPeriodicLogging::obtainConfig, 
		                                       "EventPeriodicLogging");

JSONConfig periodicLoggingJSONConfig(R"JSON(
        "EventPeriodicLogging": {
            "depends": null,
            "params": [
                [ "periodiclogging.interval", -1 ],
                [ "periodiclogging.outfile.logperiodic", "${SIMPACT_OUTPUT_PREFIX}periodiclog.csv" ]
            ],
            "info": [
                "During the simulation, at regular time intervals certain extra information",
                "can be logged with this event. Set the interval value to positive to enable,",
                "otherwise it will be disabled."
            ]
        })JSON");

