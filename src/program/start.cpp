#include "simpactpopulation.h"
#include <iostream>

using namespace std;

SimpactPopulation *createSimpactPopulation(const SimpactPopulationConfig &popConfig, const PopulationDistribution &popDist,
		                                   bool parallel, GslRandomNumberGenerator *pRng)
{
	SimpactPopulation *pPop = new SimpactPopulation(parallel, pRng);
	if (!pPop->init(popConfig, popDist))
	{
		cerr << "Unable to initialize population: " << pPop->getErrorString() << endl;
		delete pPop;
		return 0;
	}
	return pPop;
}
