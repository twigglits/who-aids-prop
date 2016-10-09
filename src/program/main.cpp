#include "gslrandomnumbergenerator.h"
#include "populationdistributioncsv.h"
#include "state.h"
#include "person.h"
#include "simpactpopulation.h"
#include "configsettings.h"
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
#include "inverseerfi.h"
#include "logsystem.h"
#include "util.h"
#include "version.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

void runHazardTests(SimpactPopulation &pop);

void usage(const string &progName)
{
	//cerr << "Usage: " << progName << " numMen numWomen parallel(0/1) tMax(years) initialInfectionFraction" << endl;
	cerr << "Usage: " << progName << " configfile.txt parallel" << endl;
	cerr << endl;
	cerr << "Version: " << SIMPACT_CYAN_VERSION << endl;
	exit(-1);
}

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
	       GslRandomNumberGenerator *pRndGen, double &tMax)
{
	LogSystem::processConfig(config);
	processNonInterventionEventConfig(config, pRndGen);
	EventIntervention::processConfig(config);
	Person::processConfig(config, pRndGen);

	int numMen, numWomen;
	double eyecapFraction;
	string ageDistFile;

	if (!config.getKeyValue("population.nummen", numMen, 0) ||
	    !config.getKeyValue("population.numwomen", numWomen, 0) ||
	    !config.getKeyValue("population.agedistfile", ageDistFile) ||
	    !config.getKeyValue("population.simtime", tMax) ||
	    !config.getKeyValue("population.eyecap.fraction", eyecapFraction, 0, 1) )
		abortWithMessage(config.getErrorString());

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

void checkConfiguration(const ConfigSettings &loadedConfig, const SimpactPopulationConfig &populationConfig, double tMax)
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

int real_main(int argc, char **argv)
{
	if (argc != 3)
		usage(argv[0]);

	string confFileName(argv[1]);
	int intParallel = atoi(argv[2]);
	ConfigSettings config;

	if (!config.load(confFileName))
	{
		cerr << "Error loading configuration file " << confFileName << endl;
		cerr << "  " << config.getErrorString() << endl;
		return -1;
	}

	GslRandomNumberGenerator rng;
	PopulationDistributionCSV ageDist(&rng);
	SimpactPopulationConfig populationConfig; // use defaults
	double tMax = -1;
	double tStart = 0;

	if (!configure(config, populationConfig, ageDist, &rng, tMax))
		return -1;

	// Check that we've used everything in the config file
	vector<string> keys;
	
	config.getUnusedKeys(keys);
	if (keys.size() != 0)
	{
		cerr << "Error: the following entries from the configuration file were not used:" << endl;
		for (int i = 0 ; i < keys.size() ; i++)
			cerr << "  " << keys[i] << endl;
		
		cerr << endl;
		return -1;
	}
	
	// Sanity check on configuration parameters
	cerr << "# Performing extra check on read configuration parameters" << endl;
	checkConfiguration(config, populationConfig, tMax);

	bool parallel = (intParallel == 1);

	SimpactPopulation pop(parallel, &rng);
	
	if (!pop.init(populationConfig, ageDist))
	{
		cerr << pop.getErrorString() << endl;
		return -1;
	}

	int64_t maxEvents = -1;
	int numInitPeople = pop.getNumberOfPeople();

	// TODO: For hazard testing! Stops after the test
#if 0
	runHazardTests(pop);
#endif

	cerr << "# Simpact version is: " << SIMPACT_CYAN_VERSION << endl;

	maxEvents = -1;
	if (!pop.run(tMax, maxEvents))
	{
		cerr << "# Error running simulation: " << pop.getErrorString() << endl;
		cerr << "# Current simulation time is " << pop.getTime() << endl;
	}

	int numEndPeople = pop.getNumberOfPeople();

	cerr << "# Number of events executed is " << maxEvents << endl;
	cerr << "# Started with " << numInitPeople << " people, ending with " << numEndPeople << " (difference is " << numEndPeople-numInitPeople << ")" << endl;

	// Log current, non-dissolved relationships
	// TODO: we only iterate over the men, since relationships are logged in lists of both men and women
	// TODO: this needs to be changed for homosexual relationships
	int numMen = pop.getNumberOfMen();
	Man **ppMen = pop.getMen();
	double infinity = numeric_limits<double>::infinity();

	for (int i = 0 ; i < numMen ; i++)
	{
		Man *pMan = ppMen[i];
		Person *pPartner = 0;
		double formationTime = 0;
		int numRelationships = pMan->getNumberOfRelationships();

		pMan->startRelationshipIteration();
		for (int j = 0 ; j < numRelationships ; j++)
		{
			pPartner = pMan->getNextRelationshipPartner(formationTime);
			assert(pPartner != 0);

			Person::writeToRelationLog(pMan, pPartner, formationTime, infinity); // infinity for not dissolved yet
		}

		double tDummy;
		assert(pMan->getNextRelationshipPartner(tDummy) == 0); // make sure the iteration is done
	}
	
	// Log different persons, both alive and deceased
	
	int numPeople = pop.getNumberOfPeople();
	Person **ppPersons = pop.getAllPeople();

	for (int i = 0 ; i < numPeople ; i++)
		ppPersons[i]->writeToPersonLog();

	// deceased
	numPeople = pop.getNumberOfDeceasedPeople();
	ppPersons = pop.getDeceasedPeople();
	
	for (int i = 0 ; i < numPeople ; i++)
		ppPersons[i]->writeToPersonLog();

	return 0;
}

int main(int argc, char **argv)
{
	InverseErfI::initialize(); // TODO: we should really 'destroy' at the end as well, to be really clean
	return real_main(argc, argv);
}
