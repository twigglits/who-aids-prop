#include "eventmortality.h"
#include "simpactpopulation.h"
#include "person.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

EventMortality::EventMortality(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventMortality::~EventMortality()
{
}

double EventMortality::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(!pPerson->hasDied());
	assert(getNumberOfPersons() == 1);

	double shape = 4.0;
	double scale = 70.0;
	double genderDiff = 5.0;

	double curTime = population.getTime();
	double ageOffset = pPerson->getAgeAt(curTime); // current age

	genderDiff /= 2.0;
	if (pPerson->getGender() == Person::Male)
		genderDiff = -genderDiff;

	scale += genderDiff;

	assert(shape > 0);
	assert(scale > 0);
	assert(ageOffset >= 0);

	return pRndGen->pickWeibull(scale, shape, ageOffset) - ageOffset; // time left to live
}

#if 1

// The 'n' in the formation hazard now is just the initial number of persons 
// and won't change, so we do not need to recalculate every event when
// someone dies and the number of people changes
int EventMortality::getNumberOfOtherAffectedPersons() const
{
	Person *pPerson = getPerson(0);
	return pPerson->getNumberOfRelationships();
}

void EventMortality::startOtherAffectedPersonIteration()
{
	Person *pPerson = getPerson(0);
	return pPerson->startRelationshipIteration();
}

PersonBase *EventMortality::getNextOtherAffectedPerson()
{
	Person *pPerson = getPerson(0);
	double t;

	return pPerson->getNextRelationshipPartner(t);
}
#else
// TODO: since the total number of people is used in the formation event
//       all formation hazards need to be recalculated
int EventMortality::getNumberOfOtherAffectedPersons() const
{
	return -1; // We're using this to recalculate everyone's events
}

void EventMortality::startOtherAffectedPersonIteration()
{
}

PersonBase *EventMortality::getNextOtherAffectedPerson()
{
	return 0;
}
#endif

std::string EventMortality::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Death of %s (current age %g)", pPerson->getName().c_str(), pPerson->getAgeAt(tNow));
	return std::string(str);

}

void EventMortality::fire(State *pState, double t)
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

		pPartner->removeRelationship(pPerson, t);
	
		// TODO: depending on how everything is implemented, the death of a 
		//       partner may cause scheduling of a formation event with someone
		//       else (not needed if formation events are already scheduled for
		//       every possible partner)

		std::cout << t << "\tDeath based dissolution between " << pPerson->getName() << " and " << pPartner->getName() << " (formed " << t-formationTime << " ago)" << std::endl;
	}

	double tDummy;
	assert(pPerson->getNextRelationshipPartner(tDummy) == 0); // make sure the iteration is done

	population.setPersonDied(pPerson);
}


