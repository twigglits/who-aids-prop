#include "simpactpopulation.h"
#include "eventmortality.h"
#include "populationdistribution.h"
#include "person.h"

SimpactPopulationConfig::SimpactPopulationConfig()
{
	m_initialMen = 100;
	m_initialWomen = 100;
}

SimpactPopulationConfig::~SimpactPopulationConfig()
{
}

SimpactPopulation::SimpactPopulation(bool parallel, GslRandomNumberGenerator *pRndGen) : Population(parallel, pRndGen)
{
	m_init = false;
}

SimpactPopulation::~SimpactPopulation()
{
}

bool SimpactPopulation::init(const SimpactPopulationConfig &config, const PopulationDistribution &popDist)
{
	if (m_init)
	{
		setErrorString("Population is already initialized");
		return false;
	}

	int numMen = config.getInitialMen();
	int numWomen = config.getInitialWomen();

	if (numMen < 0 || numWomen < 0)
	{
		setErrorString("The number of men and women must be at least zero");
		return false;
	}

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

	// Start with the events
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
		onNewEvent(pEvt);
	}
}

void SimpactPopulation::onAboutToFire(EventBase *pEvt)
{
	PopulationEvent *pEvent = static_cast<PopulationEvent *>(pEvt);

	double t = getTime();
	std::cout << t << "\t" << pEvent->getDescription(t) << std::endl;
}

