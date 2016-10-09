#include "configutil.h"
#include "eventaidsmortality.h"
#include "eventchronicstage.h"
#include "eventdebut.h"
#include "eventdissolution.h"
#include "eventformation.h"
#include "eventmortality.h"
#include "eventmortalitybase.h"
#include "eventtransmission.h"
#include "eventtreatment.h"
#include "eventhivseed.h"
#include "eventaidsstage.h"
#include "eventintervention.h"
#include "eventconception.h"
#include "eventbirth.h"
#include "logsystem.h"
#include "util.h"
#include "populationdistributioncsv.h"
#include "gslrandomnumbergenerator.h"
#include <vector>

using namespace std;

void processNonInterventionEventConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventAIDSMortality::processConfig(config);
	EventChronicStage::processConfig(config);
	EventDebut::processConfig(config);
	EventDissolution::processConfig(config);
	EventFormation::processConfig(config);
	EventMortality::processConfig(config);
	EventTransmission::processConfig(config);
	EventTreatment::processConfig(config);
	EventHIVSeed::processConfig(config);
	EventAIDSStage::processConfig(config);
	EventConception::processConfig(config, pRndGen);
	EventBirth::processConfig(config, pRndGen);
}

bool configure(ConfigSettings &config, SimpactPopulationConfig &populationConfig, PopulationDistributionCSV &ageDist,
	       GslRandomNumberGenerator *pRndGen, double &tMax, int64_t &maxEvents)
{
	LogSystem::processConfig(config);
	processNonInterventionEventConfig(config, pRndGen);
	EventIntervention::processConfig(config);
	Person::processConfig(config, pRndGen);

	int numMen, numWomen;
	double eyecapFraction;
	string ageDistFile;

	cout << "maxEvents " << maxEvents << endl;

	if (!config.getKeyValue("population.nummen", numMen, 0) ||
	    !config.getKeyValue("population.numwomen", numWomen, 0) ||
	    !config.getKeyValue("population.agedistfile", ageDistFile) ||
	    !config.getKeyValue("population.simtime", tMax) ||
	    !config.getKeyValue("population.maxevents", maxEvents) ||
	    !config.getKeyValue("population.eyecap.fraction", eyecapFraction, 0, 1) )
		abortWithMessage(config.getErrorString());

	cout << "maxEvents " << maxEvents << endl;

	populationConfig.setInitialMen(numMen);
	populationConfig.setInitialWomen(numWomen);
	populationConfig.setEyeCapsFraction(eyecapFraction);

	if (!ageDist.load(ageDistFile))
	{
		cerr << "Can't load age distribution data: " << ageDist.getErrorString() << endl;
		return false;
	}

	return true;
}

bool areValuesCompatible(const string &key, const std::string &A, const std::string &B, bool canIgnore = true) // B can be 'IGNORE', just print a warning then
{
	if (canIgnore)
	{
		if (B == "IGNORE")
		{
			cerr << "# WARNING: ignoring consistency check for config key " << key << " (config value is '" << A << "')" << endl;
			return true;
		}
	}

	if (A == B) // if the strings are exactly the same, it should be ok
		return true;

	int num1, num2;
	if (parseAsInt(A, num1) && parseAsInt(B, num2))
	{
		if (num1 == num2)
			return true;
		return false;
	}

	double x1, x2;
	if (parseAsDouble(A, x1) && parseAsDouble(B, x2))
	{
		if (x1 == x2)
			return true;

		double diff;

		if (x1 == 0)
			diff = std::abs(x2);
		else if (x2 == 0)
			diff = std::abs(x1);
		else
			diff = 0.5*(std::abs((x1-x2)/x1) + std::abs((x1-x2)/x2));

		if (diff < 1e-10)
		{
			cerr << "# WARNING: ignoring small (" << diff << ") difference between " << A << " and " << B << " in key " << key << endl;
			return true;
		}
		cerr << "# ERROR: relative difference between two doubles (" << A << " and " << B << ") is too large: " << diff << endl;
		return false;
	}

	// At this point, it's still possible that we're dealing with a comma separated list of
	// values, as in the intervention.times settings

	vector<string> partsA;
	vector<string> partsB;

	SplitLine(A, partsA, ",");
	SplitLine(B, partsB, ",");

	if (partsA.size() != partsB.size())
		return false;

	if (partsA.size() == 1) // In this case, we've already checked the contents
		return false;

	// More than one part, check every one of them
	for (int i = 0 ; i < partsA.size() ; i++)
	{
		// Individual parts must be compatible

		string subA = trim(partsA[i]);
		string subB = trim(partsB[i]);

		if (!areValuesCompatible(key, subA, subB, false))
			return false;
	}

	// The comma separated lists are compatible
	return true;
}

void checkConfiguration(const ConfigSettings &loadedConfig, const SimpactPopulationConfig &populationConfig, double tMax,
		        int64_t maxEvents)
{
	ConfigWriter config;

	LogSystem::obtainConfig(config);
	EventAIDSMortality::obtainConfig(config);
	EventChronicStage::obtainConfig(config);
	EventDebut::obtainConfig(config);
	EventDissolution::obtainConfig(config);
	EventFormation::obtainConfig(config);
	EventMortality::obtainConfig(config);
	EventTransmission::obtainConfig(config);
	EventTreatment::obtainConfig(config);
	EventHIVSeed::obtainConfig(config);
	EventAIDSStage::obtainConfig(config);
	EventConception::obtainConfig(config);
	EventBirth::obtainConfig(config);
	EventIntervention::obtainConfig(config);
	Person::obtainConfig(config);

	if (!config.addKey("population.nummen", populationConfig.getInitialMen()) ||
	    !config.addKey("population.numwomen", populationConfig.getInitialWomen()) ||
	    !config.addKey("population.agedistfile", "IGNORE") || // not going to check file contents
	    !config.addKey("population.simtime", tMax) ||
	    !config.addKey("population.maxevents", maxEvents) ||
	    !config.addKey("population.eyecap.fraction", populationConfig.getEyeCapsFraction()) )
		abortWithMessage(config.getErrorString());

	vector<string> keys;

	loadedConfig.getKeys(keys);
	for (int i = 0 ; i < keys.size() ; i++)
	{
		string val1, val2;
		bool dummy;

		if (!loadedConfig.getStringKeyValue(keys[i], val1, dummy))
			abortWithMessage("INTERNAL ERROR: can't get a value for a reported key");

		if (!config.getKeyValue(keys[i], val2))
			abortWithMessage("Consistency error: " + keys[i] + " is present in config file but doesn't seem to be configured");
	
		//cerr << "Checking " << keys[i] << ":" << val1 << " vs " << val2 << endl;
		if (!areValuesCompatible(keys[i], val1, val2))
			abortWithMessage("Consistency error: inconsistency for key " + keys[i] + " (" + val1 + " <-> " + val2 + ")");
	}

	config.getKeys(keys);
	for (int i = 0 ; i < keys.size() ; i++)
	{
		string val1, val2;
		bool dummy;

		if (!config.getKeyValue(keys[i], val2))
			abortWithMessage("INTERNAL ERROR: can't get a value for a reported key");

		if (!loadedConfig.getStringKeyValue(keys[i], val1, dummy))
			abortWithMessage("Consistency error: " + keys[i] + " is configured but not present in config file");

		// If all keys are the same, we've already checked the contents for compatibility
	}
}

extern const char configJSon[];

void showConfigOptions()
{
	cout << configJSon << endl;
}

