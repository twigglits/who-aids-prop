#include "eventmortalitybase.h"
#include "eventmortality.h"
#include "eventdebut.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

using namespace std;

EventMortalityBase::EventMortalityBase(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventMortalityBase::~EventMortalityBase()
{
}

void EventMortalityBase::markOtherAffectedPeople(const Population &population)
{
	Person *pPerson = getPerson(0);
	assert(pPerson != 0);

	int numRelationships = pPerson->getNumberOfRelationships();
	pPerson->startRelationshipIteration();

	for  (int i = 0 ; i < numRelationships ; i++)
	{
		double formationTime;
		Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
		population.markAffectedPerson(pPartner);
	}

	double tDummy;
	assert(pPerson->getNextRelationshipPartner(tDummy) == 0); // make sure the iteration is done
}

void EventMortalityBase::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);

	Person *pPerson = getPerson(0);
	assert(pPerson != 0);

	int numRelationships = pPerson->getNumberOfRelationships();
	pPerson->startRelationshipIteration();

	for  (int i = 0 ; i < numRelationships ; i++)
	{
		double formationTime;
		Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);

		assert(pPartner != 0);
		assert(!pPartner->hasDied());

		pPartner->removeRelationship(pPerson, t, true);
	
		// TODO: depending on how everything is implemented, the death of a 
		//       partner may cause scheduling of a formation event with someone
		//       else (not needed if formation events are already scheduled for
		//       every possible partner)

		//This is written to a log because of code in removeRelationship
		//cout << t << "\tDeath based dissolution between " << pPerson->getName() << " and " << pPartner->getName() << " (formed " << t-formationTime << " ago)" << endl;
	}

	double tDummy;
	assert(pPerson->getNextRelationshipPartner(tDummy) == 0); // make sure the iteration is done

	population.setPersonDied(pPerson);
}

