#include "eventintervention.h"
#include "gslrandomnumbergenerator.h"
#include "util.h"
#include "configsettings.h"
#include <stdio.h>
#include <iostream>

using namespace std;

void processNonInterventionEventConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);

EventIntervention::EventIntervention()
{
}

EventIntervention::~EventIntervention()
{
}

double EventIntervention::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);

	double evtTime = getNextInterventionTime();
	double dt = evtTime - population.getTime();

	if (evtTime < 0 || dt < 0)
		abortWithMessage("EventIntervention::getNextInterventionTime: the next intervention takes place at time " + doubleToString(evtTime) + " which is before the current time " + doubleToString(population.getTime()) + " (dt = " + doubleToString(dt) + ")"); 
	
	return dt;
}

std::string EventIntervention::getDescription(double tNow) const
{
	return std::string("Intervention event");
}

void EventIntervention::writeLogs(double tNow) const
{
	writeEventLogStart(true, "intervention", tNow, 0, 0);
}

void EventIntervention::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	double interventionTime;
	ConfigSettings interventionConfig;

	popNextInterventionInfo(interventionTime, interventionConfig);
	assert(interventionTime == t); // make sure we're at the correct time

	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	// Re-read the event configs
	processNonInterventionEventConfig(interventionConfig, pRndGen);
	// Re-read the person config
	Person::processConfig(interventionConfig, pRndGen);

	if (EventIntervention::hasNextIntervention()) // check if we need to schedule a next intervention
	{
		EventIntervention *pEvt = new EventIntervention();
		population.onNewEvent(pEvt);
	}
}

void EventIntervention::processConfig(ConfigSettings &config)
{
	// This event can only be initialized once!
	if (m_interventionsProcessed)
		abortWithMessage("Intervention event has already been initialized!");
	m_interventionsProcessed = true;

	// check the config file
	vector<string> yesNoOptions;
	string yesNo;

	yesNoOptions.push_back("yes");
	yesNoOptions.push_back("no");

	if (!config.getKeyValue("intervention.enabled", yesNo, yesNoOptions))
		abortWithMessage(config.getErrorString());

	if (yesNo == "no") // no interventions, nothing to do
		return;

	string baseConfigName, timeStr, fileIDs;

	if (!config.getKeyValue("intervention.baseconfigname", baseConfigName) ||
	    !config.getKeyValue("intervention.times", timeStr) ||
	    !config.getKeyValue("intervention.fileids", fileIDs) )
		abortWithMessage(config.getErrorString());
	
	baseConfigName = trim(baseConfigName);
	if (baseConfigName.length() == 0)
		abortWithMessage("You need to specify a base config file name");

	vector<string> timeStrParts, timeStrPartsTrimmed;
	vector<string> fileIDParts;
	list<double> interventionTimes;

	timeStr = trim(timeStr);
	SplitLine(timeStr, timeStrParts, ",", "", "", false);

	for (int i = 0 ; i < timeStrParts.size() ; i++)
	{
		string valueStr = trim(timeStrParts[i]);
		double t;

		if (!parseAsDouble(valueStr, t))
			abortWithMessage("Can't interpret '" + valueStr + "' as a number in intervention.times");

		interventionTimes.push_back(t);
		timeStrPartsTrimmed.push_back(valueStr);
	}

	fileIDs = trim(fileIDs);
	if (fileIDs.length() == 0) // empty fileIDs line means the time strings should be used as IDs
		fileIDParts = timeStrPartsTrimmed;
	else
		SplitLine(fileIDs, fileIDParts, ",", "", "", false);

	if (fileIDParts.size() != timeStrParts.size())
		abortWithMessage("The number of fileIDs does not match the number of intervention times");

	ConfigSettings baseSettings = config; // we'll let each intervention config start from the previous setting
 
	assert(m_interventionSettings.size() == 0);
	assert(m_interventionTimes.size() == 0);

	// Ok, got everything we need. Load the config files.
	for (int i = 0 ; i < fileIDParts.size() ; i++)
	{
		string fileName = replace(baseConfigName, "%", fileIDParts[i]);
		ConfigSettings interventionSettings;

		if (!interventionSettings.load(fileName))
			abortWithMessage("Can't configure intervention event: " + interventionSettings.getErrorString());

		baseSettings.merge(interventionSettings);
		baseSettings.clearUsageFlags();

		m_interventionSettings.push_back(baseSettings);
	}

	double prevTime = 0; // all intervention times must be positive and increasing
	for (list<double>::const_iterator it = interventionTimes.begin() ; it != interventionTimes.end() ; it++)
	{
		double t = *it;

		if (t < prevTime)
			abortWithMessage("Times must be positive and increasing in the intervention settings");

		prevTime = t;
	}

	m_interventionTimes = interventionTimes;
}

void EventIntervention::obtainConfig(ConfigWriter &config)
{
	if (m_interventionTimes.size() == 0)
	{
		if (!config.addKey("intervention.enabled", "no"))
			abortWithMessage(config.getErrorString());
		return;
	}

	// Here, we will only store the intervention times, ignoring the files

	if (!config.addKey("intervention.enabled", "yes") ||
	    !config.addKey("intervention.baseconfigname", "IGNORE") ||
	    !config.addKey("intervention.fileids", "IGNORE") )
		abortWithMessage(config.getErrorString());

	list<double>::const_iterator it = m_interventionTimes.begin();
	string timeStr = doubleToString(*it);

	it++;
	while (it != m_interventionTimes.end())
	{
		timeStr += "," + doubleToString(*it);
		it++;
	}

	if (!config.addKey("intervention.times", timeStr))
		abortWithMessage(config.getErrorString());
}

list<double> EventIntervention::m_interventionTimes;
list<ConfigSettings> EventIntervention::m_interventionSettings;
bool EventIntervention::m_interventionsProcessed = false;

bool EventIntervention::hasNextIntervention()
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	if (m_interventionTimes.size() > 0)
		return true;
	return false;
}

double EventIntervention::getNextInterventionTime()
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	assert(m_interventionTimes.size() > 0);
	return *(m_interventionTimes.begin());
}

void EventIntervention::popNextInterventionInfo(double &t, ConfigSettings &config)
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	assert(m_interventionTimes.size() > 0);

	t = *(m_interventionTimes.begin());
	config = *(m_interventionSettings.begin());

	m_interventionTimes.pop_front();
	m_interventionSettings.pop_front();
}

