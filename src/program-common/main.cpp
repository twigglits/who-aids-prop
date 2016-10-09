#include "gslrandomnumbergenerator.h"
#include "populationdistributioncsv.h"
#include "person.h"
#include "simpactpopulation.h"
#include "configsettings.h"
#include "inverseerfi.h"
#include "version.h"
#include "configutil.h"
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
	cerr << "Usage: " << progName << " configfile.txt parallel" << endl << endl;;
	cerr << "or" << endl;
	cerr << "Usage: " << progName << " --showconfigoptions" << endl << endl;;
	cerr << endl;
	cerr << "Version: " << SIMPACT_CYAN_VERSION << endl;
	exit(-1);
}

int real_main(int argc, char **argv)
{
	if (argc == 2)
	{
		string flag = argv[1];
		if (flag == "--showconfigoptions")
		{
			showConfigOptions();
			return 0;
		}
	}
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
	int64_t maxEvents = -1;

	if (!configure(config, populationConfig, ageDist, &rng, tMax, maxEvents))
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
	checkConfiguration(config, populationConfig, tMax, maxEvents);

	bool parallel = (intParallel == 1);

	SimpactPopulation pop(parallel, &rng);
	
	if (!pop.init(populationConfig, ageDist))
	{
		cerr << pop.getErrorString() << endl;
		return -1;
	}

	int numInitPeople = pop.getNumberOfPeople();

	// TODO: For hazard testing! Stops after the test
#if 0
	runHazardTests(pop);
#endif

	cerr << "# Simpact version is: " << SIMPACT_CYAN_VERSION << endl;

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
	{
		Person *pPerson = ppPersons[i];

		pPerson->writeToPersonLog();
		if (pPerson->isInfected() && pPerson->hasLoweredViralLoad())
			pPerson->writeToTreatmentLog(infinity, false);
	}

	// deceased
	numPeople = pop.getNumberOfDeceasedPeople();
	ppPersons = pop.getDeceasedPeople();
	
	for (int i = 0 ; i < numPeople ; i++)
		ppPersons[i]->writeToPersonLog();

	return 0;
}

int main(int argc, char **argv)
{
//	InverseErfI::initialize(); // TODO: we should really 'destroy' at the end as well, to be really clean
	return real_main(argc, argv);
}
