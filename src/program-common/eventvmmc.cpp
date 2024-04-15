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

EventVMMC::EventVMMC(Person *pPerson) : SimpactEvent(pPerson)
{
	Person *pPerson = getPerson(0);  //we instantiate the person class
}

EventVMMC::~EventVMMC()
{
}

double EventVMMC::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);

	double evtTime = getNextInterventionTime();
	double dt = evtTime - population.getTime();

	if (evtTime < 0 || dt < 0)
		abortWithMessage("EventVMMC::getNextInterventionTime: the next VMMC intervention takes place at time " + doubleToString(evtTime) + " which is before the current time " + doubleToString(population.getTime()) + " (dt = " + doubleToString(dt) + ")"); 
	
	return dt;
}

string EventVMMC::getDescription(double tNow) const
{
	return strprintf("VMMC event for %s", getPerson(0)->getName().c_str());
}

void EventVMMC::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "VMMC", tNow, pPerson, 0);  //when set to true we write logs
}

bool EventVMMC::isEligibleForTreatment(double t)
{
    // Person must be male and 15 years or older to be eligible for treatment.
    Man *pMale = MAN(getPerson(0));
    assert(pMale->isMan());   // we assert that a person is from the male class
    
    double age = pMale->getAgeAt(t);
    if (pMale->isMan() && !pMale->isVmmc() && age >= 15.0) {  //if person is male & not yet circumsized & age 15 or older
        return true;  // eligible for treatment
    } else {
        return false; // not eligible for treatment
    }
}

bool EventVMMC::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen) {
	Person *pPerson = getPerson(0);

	double x = pRndGen->pickRandomDouble();
	if (x < pPerson->hiv().getARTAcceptanceThreshold())
		return true;

	return false;
}

void EventVMMC::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	double interventionTime;
	ConfigSettings interventionConfig;

	popNextInterventionInfo(interventionTime, interventionConfig);
	assert(interventionTime == t); // make sure we're at the correct time
	
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pPerson = getPerson(0);
	
	if (isEligibleForTreatment(t) && isWillingToStartTreatment(t, pRndGen))  //so if this condition is met we set the VMMC property to true
	{
		SimpactEvent::writeEventLogStart(true, "(VMMC_treatment)", t, pPerson, 0);

		Man *pMale = MAN(pPerson);
        assert(pMale->isMan()); // Ensure the person is a man
        
        // Set the isVmmc property to true
        pMale->setVmmc(true);
	}
	population.initializeFormationEvents(pPerson, false, false, t);
}

// double EventVMMC:: = -1;
// PieceWiseLinearFunction *EventVMMC::s_pRecheckInterval = 0;

void EventVMMC::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	// This event can only be initialized once!
	if (m_interventionsProcessed)
		abortWithMessage("Intervention event has already been initialized!");
	m_interventionsProcessed = true;

	// check the config file
	vector<string> yesNoOptions;
	string yesNo;
	bool_t r;

	yesNoOptions.push_back("yes");
	yesNoOptions.push_back("no");

	if (!(r = config.getKeyValue("intervention.enabled", yesNo, yesNoOptions)))
		abortWithMessage(r.getErrorString());

	if (yesNo == "no") // no interventions, nothing to do
		return;

	string baseConfigName, timeStr, fileIDs;

	if (!(r = config.getKeyValue("intervention.baseconfigname", baseConfigName)) ||
	    !(r = config.getKeyValue("intervention.times", timeStr)) ||
	    !(r = config.getKeyValue("intervention.fileids", fileIDs)) )
		abortWithMessage(r.getErrorString());
	
	baseConfigName = trim(baseConfigName);
	if (baseConfigName.length() == 0)
		abortWithMessage("You need to specify a base config file name");

	vector<string> timeStrParts, timeStrPartsTrimmed;
	vector<string> fileIDParts;
	list<double> interventionTimes;

	timeStr = trim(timeStr);
	SplitLine(timeStr, timeStrParts, ",", "", "", false);

	for (size_t i = 0 ; i < timeStrParts.size() ; i++)
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
	for (size_t i = 0 ; i < fileIDParts.size() ; i++)
	{
		string fileName = replace(baseConfigName, "%", fileIDParts[i]);
		ConfigSettings interventionSettings;

		if (!(r = interventionSettings.load(fileName)))
			abortWithMessage("Can't configure intervention event: " + r.getErrorString());

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

void EventVMMC::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (m_interventionTimes.size() == 0)
	{
		if (!(r = config.addKey("intervention.enabled", "no")))
			abortWithMessage(r.getErrorString());
		return;
	}

	// Here, we will only store the intervention times, ignoring the files

	if (!(r = config.addKey("intervention.enabled", "yes")) ||
	    !(r = config.addKey("intervention.baseconfigname", "IGNORE")) ||
	    !(r = config.addKey("intervention.fileids", "IGNORE")) )
		abortWithMessage(r.getErrorString());

	list<double>::const_iterator it = m_interventionTimes.begin();
	string timeStr = doubleToString(*it);

	it++;
	while (it != m_interventionTimes.end())
	{
		timeStr += "," + doubleToString(*it);
		it++;
	}

	if (!(r = config.addKey("intervention.times", timeStr)))
		abortWithMessage(r.getErrorString());
}

list<double> EventVMMC::m_interventionTimes;
list<ConfigSettings> EventVMMC::m_interventionSettings;
bool EventVMMC::m_interventionsProcessed = false;

bool EventVMMC::hasNextIntervention()
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	if (m_interventionTimes.size() > 0)
		return true;
	return false;
}

double EventVMMC::getNextInterventionTime()
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	assert(m_interventionTimes.size() > 0);
	return *(m_interventionTimes.begin());
}

void EventVMMC::popNextInterventionInfo(double &t, ConfigSettings &config)
{
	assert(m_interventionTimes.size() == m_interventionSettings.size());
	assert(m_interventionTimes.size() > 0);

	t = *(m_interventionTimes.begin());
	config = *(m_interventionSettings.begin());

	m_interventionTimes.pop_front();
	m_interventionSettings.pop_front();
}

ConfigFunctions interventionConfigFunctions(EventVMMC::processConfig, EventVMMC::obtainConfig,
		                                    "EventIntervention", "initonce");

JSONConfig interventionJSONConfig(R"JSON(
        "EventIntervention": { 
            "depends": null,
            "params": [ ["intervention.enabled", "no", [ "yes", "no"] ] ],
            "info": [ 
                "If you enable the intervention event, you need to specify a number of times",
                "at which this event should fire. On these times, some new configuration lines",
                "will be read, overriding the initial parameters read from config file."
            ]
        },

        "EventIntervention_enabled": { 
            "depends": [ "EventIntervention", "intervention.enabled", "yes"],
            "params": [ 
                 ["intervention.baseconfigname", null],
                 ["intervention.times", null],
                 ["intervention.fileids", null] ],
            "info": [ 
                "In 'intervention.times' you need to specify the times at which the ",
                "intervention event should fire. All times must be positive and the list",
                "of times must be increasing.",
                "",
                "The 'intervention.baseconfigname' is the filename template that should be",
                "used to read the config settings from for the intervention events. For each",
                "intervention time, the '%' character will either be replaced by the ",
                "corresponding string from 'intervention.fileids', or by the time specified ",
                "in 'intervention.times' if you leave 'intervention.fileids' empty.",
                "",
                "For example:",
                "  intervention.baseconfigname = intconfig_%.txt",
                "  intervention.times = 1,2,3",
                "  intervention.fileids = A,B,C",
                "will read intervention settings from 'intconfig_A.txt', 'intconfig_B.txt' and",
                "'intconfig_C.txt'.",
                "",
                "If you leave the file IDs empty,",
                "  intervention.fileids =",
                "then the files used would be 'intconfig_1.txt', 'intconfig_2.txt' and",
                "'intconfig_3.txt'."
            ]
        })JSON");


