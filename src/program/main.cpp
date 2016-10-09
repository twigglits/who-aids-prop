#include "gslrandomnumbergenerator.h"
#include "populationdistributioncsv.h"
#include "state.h"
#include "person.h"
#include "simpactpopulation.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace std;

void usage(const string &progName)
{
	cerr << "Usage: " << progName << " numMen numWomen parallel(0/1) tMax(years) initialInfectionFraction" << endl;
	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc != 6)
		usage(argv[0]);

	int numMen = atoi(argv[1]);
	int numWomen = atoi(argv[2]);
	int intParallel = atoi(argv[3]);
	double tMax = strtod(argv[4], 0);
	double initialInfectionFraction = strtod(argv[5], 0);

	GslRandomNumberGenerator rng;
	PopulationDistributionCSV ageDist(&rng);

	// TODO: which interpretation of csv file?
	//if (!ageDist.load("../data/sa_2013.csv"))
	if (!ageDist.load("../data/sa_2003.csv"))
	{
		cerr << "Can't load age distribution data: " << ageDist.getErrorString() << endl;
		return -1;
	}

	SimpactPopulationConfig config; // use defaults

	config.setInitialMen(numMen);
	config.setInitialWomen(numWomen);
	config.setInitialInfectionFraction(initialInfectionFraction);

	bool parallel = (intParallel == 1);

	SimpactPopulation pop(parallel, &rng);
	
	if (!pop.init(config, ageDist))
	{
		cerr << pop.getErrorString() << endl;
		return -1;
	}

	int64_t maxEvents = -1;
	int numInitPeople = pop.getNumberOfPeople();

	if (!pop.run(tMax, maxEvents))
	{
		cerr << "# Error running simulation: " << pop.getErrorString() << endl;
		cerr << "# Current simulation time is " << pop.getTime() << endl;
	}

	int numEndPeople = pop.getNumberOfPeople();

	cerr << "# Number of events executed is " << maxEvents << endl;
	cerr << "# Started with " << numInitPeople << " people, ending with " << numEndPeople << " (difference is " << numEndPeople-numInitPeople << ")" << endl;

	return 0;
}
