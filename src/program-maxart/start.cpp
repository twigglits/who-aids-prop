#include "maxartpopulation.h"
#include <iostream>

using namespace std;

SimpactPopulation *createSimpactPopulation(const SimpactPopulationConfig &popConfig, const PopulationDistribution &popDist,
		                                   bool parallel, GslRandomNumberGenerator *pRng)
{
	MaxARTPopulation *pPop = new MaxARTPopulation(parallel, pRng);
	if (!pPop->init(popConfig, popDist))
	{
		cerr << "Unable to initialize population: " << pPop->getErrorString() << endl;
		delete pPop;
		return 0;
	}
	return pPop;
}
