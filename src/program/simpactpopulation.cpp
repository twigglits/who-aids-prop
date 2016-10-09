#include "simpactpopulation.h"
#include "eventmortality.h"
#include "eventformation.h"
#include "eventdebut.h"
#include "eventhivtest.h"
#include "eventchronicstage.h"
#include "populationdistribution.h"
#include "person.h"
#include "gslrandomnumbergenerator.h"

SimpactPopulationConfig::SimpactPopulationConfig()
{
	// TODO: init default values!
	
	m_initialMen = 100;
	m_initialWomen = 100;
	m_initialInfectionFraction = 0;
}

SimpactPopulationConfig::~SimpactPopulationConfig()
{
}

SimpactPopulation::SimpactPopulation(bool parallel, GslRandomNumberGenerator *pRndGen) : Population(parallel, pRndGen)
{
	m_initialPopulationSize = -1;
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

	GslRandomNumberGenerator *pRngGen = getRandomNumberGenerator();
	double initialInfectionFraction = config.getInitialInfectionFraction();

	// Time zero is at the start of the simulation, so the birth dates are negative

	for (int i = 0 ; i < numMen ; i++)
	{
		double age = popDist.pickAge(true);

		Person *pPerson = new Man(-age);

		if (age > getDebutAge())
			pPerson->setSexuallyActive();

		if (pRngGen->pickRandomDouble() < initialInfectionFraction)
			pPerson->setInfected(0, 0, Person::Seed); // TODO: what about the infection time here?

		addNewPerson(pPerson);
	}
	
	for (int i = 0 ; i < numWomen ; i++)
	{
		double age = popDist.pickAge(false);

		Person *pPerson = new Woman(-age);
		if (age > getDebutAge())
			pPerson->setSexuallyActive();

		if (pRngGen->pickRandomDouble() < initialInfectionFraction)
			pPerson->setInfected(0, 0, Person::Seed); // TODO: what about the infection time here?

		addNewPerson(pPerson);
	}

	m_initialPopulationSize = numMen + numWomen;

	// Start with the events

	onScheduleInitialEvents();

	m_init = true;
	return true;
}

void SimpactPopulation::onScheduleInitialEvents()
{
	int numMen = getNumberOfMen();
	int numWomen = getNumberOfWomen();
	int numPeople = getNumberOfPeople();

	Man **ppMen = getMen();
	Woman **ppWomen = getWomen();
	Person **ppPeople = getAllPeople();

	// Initialize the event list with the mortality events
	// Depending on the infection status of the person, a different
	// kind of mortality will be used
	for (int i = 0 ; i < numPeople ; i++)
	{
		EventMortality *pEvt = new EventMortality(ppPeople[i]);
		onNewEvent(pEvt);
	}

	// For the people who are infected, schedule an HIV test event
	// TODO: do we need to add some randomness here to avoid all these
	// events firing on the same day?

	for (int i = 0 ; i < numPeople ; i++)
	{
		if (ppPeople[i]->isInfected())
		{
			EventHIVTest *pEvt = new EventHIVTest(ppPeople[i]);
			onNewEvent(pEvt);
		}
	}

	// Check if we're in the acute stage, in that case we should schedule
	// an event to mark the transition to the chronic stage

	for (int i = 0 ; i < numPeople ; i++)
	{
		if (ppPeople[i]->isInfected())
		{
			double infTime = ppPeople[i]->getInfectionTime();
			double curTime = 0; // we're at time zero

			if (curTime-infTime >= getAcuteStageTime()) // we're past the acute stage, set a flag in the person
				ppPeople[i]->setInChronicStage();
			else
			{
				// we're still in the acute stage and should schedule
				// an event to mark the transition to the chronic stage

				EventChronicStage *pEvtChronic = new EventChronicStage(ppPeople[i]);
				onNewEvent(pEvtChronic);
			}
		}
	}

	// Relationship formation (every active man with every active woman)

	for (int i = 0 ; i < numWomen ; i++)
	{
		Woman *pWoman = ppWomen[i];
		assert(pWoman->getGender() == Person::Female);

		if (pWoman->isSexuallyActive())
		{
			for (int j = 0 ; j < numMen ; j++)
			{
				Man *pMan = ppMen[j];
				assert(pMan->getGender() == Person::Male);
		
				if (pMan->isSexuallyActive())
				{
					EventFormation *pEvt = new EventFormation(pMan, pWoman, -1);
					onNewEvent(pEvt);
				}
			}
		}
	}
	
	// For the people who are not sexually active, set a debut event

	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];

		if (!pPerson->isSexuallyActive())
		{
			EventDebut *pEvt = new EventDebut(pPerson);
			onNewEvent(pEvt);
		}
	}
}

void SimpactPopulation::onAboutToFire(EventBase *pEvt)
{
	PopulationEvent *pEvent = static_cast<PopulationEvent *>(pEvt);

	double t = getTime();
	std::cout << t << "\t" << pEvent->getDescription(t) << std::endl;
}

