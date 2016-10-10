#include "simpactpopulation.h"
#include "eventmortality.h"
#include "populationdistribution.h"
#include "person.h"
#include "populationalgorithmadvanced.h"
#include "populationalgorithmsimple.h"
#include "populationstatesimple.h"
#include "populationstateadvanced.h"
#include <iostream>

using namespace std;

SimpactPopulationConfig::SimpactPopulationConfig()
{
	m_initialMen = 100;
	m_initialWomen = 100;
}

SimpactPopulationConfig::~SimpactPopulationConfig()
{
}

SimpactPopulation::SimpactPopulation(PopulationAlgorithmInterface &alg, PopulationStateInterface &state) 
	: PopulationStateExtra(), m_state(state), m_alg(alg)
{
	m_init = false;
	state.setExtraStateInfo(this);
	alg.setAboutToFireAction(this);
}

SimpactPopulation::~SimpactPopulation()
{
}

bool_t SimpactPopulation::init(const SimpactPopulationConfig &config, const PopulationDistribution &popDist)
{
	if (m_init)
		return "Population is already initialized";

	int numMen = config.getInitialMen();
	int numWomen = config.getInitialWomen();

	if (numMen < 0 || numWomen < 0)
		return "The number of men and women must be at least zero";

	// Time zero is at the start of the simulation, so the birth dates are negative

	for (int i = 0 ; i < numMen ; i++)
	{
		double age = popDist.pickAge(true);

		Person *pPerson = new Man(-age);
		addNewPerson(pPerson);
	}
	
	for (int i = 0 ; i < numWomen ; i++)
	{
		double age = popDist.pickAge(false);

		Person *pPerson = new Woman(-age);
		addNewPerson(pPerson);
	}

	// Start with the event
	onScheduleInitialEvents();

	m_init = true;
	return true;
}

void SimpactPopulation::onScheduleInitialEvents()
{
	int numPeople = getNumberOfPeople();
	Person **ppPeople = getAllPeople();

	// Initialize the event list with the mortality events
	for (int i = 0 ; i < numPeople ; i++)
	{
		EventMortality *pEvt = new EventMortality(ppPeople[i]);
		m_alg.onNewEvent(pEvt);
	}
}

void SimpactPopulation::onAboutToFire(PopulationEvent *pEvent)
{
	double t = getTime();
	cout << t << "\t" << pEvent->getDescription(t) << endl;
}

bool_t selectAlgorithmAndState(const string &algo, GslRandomNumberGenerator &rng, bool parallel,
		                     PopulationAlgorithmInterface **ppAlgo, PopulationStateInterface **ppState)
{
	if (algo == "opt")
	{
		PopulationStateAdvanced *pPopState = new PopulationStateAdvanced();
		*ppState = pPopState;
		*ppAlgo = new PopulationAlgorithmAdvanced(*pPopState, rng, parallel);
	}
	else if (algo == "simple")
	{
		PopulationStateSimple *pPopState = new PopulationStateSimple();
		*ppState = pPopState;
		*ppAlgo = new PopulationAlgorithmSimple(*pPopState, rng, parallel);
	}
	else
		return "Invalid algorithm: " + algo;

	bool_t r = (*ppAlgo)->init();
	if (!r)
	{
		delete *ppState;
		delete *ppAlgo;
		return r;
	}

	return true;
}

