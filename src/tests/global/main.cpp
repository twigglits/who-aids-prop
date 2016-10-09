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

class PopulationDistributionConstant : public PopulationDistribution
{
public:
	PopulationDistributionConstant(double ageMale, double ageFemale, GslRandomNumberGenerator *pRnd) : PopulationDistribution(pRnd)
	{
		m_ageMale = ageMale;
		m_ageFemale = ageFemale;
	}

	~PopulationDistributionConstant()
	{
	}

	double pickAge(bool male) const
	{
		if (male)
			return m_ageMale;

		return m_ageFemale;
	}
private:
	double m_ageMale, m_ageFemale;
};

void usage(const std::string &progName)
{
	std::cerr << "Usage: " << progName << " numMen numWomen parallel(0/1)" << std::endl;
	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
		usage(argv[0]);

	int numMen = atoi(argv[1]);
	int numWomen = atoi(argv[2]);
	int intParallel = atoi(argv[3]);
	double tMax = 1e200; // we're just going to run this until everyone is dead

	GslRandomNumberGenerator rng;
	SimpactPopulationConfig config;

	std::cerr << "# WARNING: using fixed starting age" << std::endl;
	PopulationDistributionConstant ageDist(30.0, 40.0, &rng);

	config.setInitialMen(numMen);
	config.setInitialWomen(numWomen);

	bool parallel = (intParallel == 1);
	SimpactPopulation pop(parallel, &rng);
	
	if (!pop.init(config, ageDist))
	{
		std::cerr << pop.getErrorString() << std::endl;
		return -1;
	}

	int64_t maxEvents = -1; // don't specify a maximum

	if (!pop.run(tMax, maxEvents))
	{
		std::cerr << "# Error running simulation: " << pop.getErrorString() << std::endl;
		std::cerr << "# Current simulation time is " << pop.getTime() << std::endl;
	}

	std::cerr << "# Number of events executed is " << maxEvents << std::endl;

	return 0;
}
