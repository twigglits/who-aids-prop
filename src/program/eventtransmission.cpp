#include "eventtransmission.h"
#include "eventconception.h"
#include "eventbirth.h"
#include "eventhivtest.h"
#include "eventmortality.h"
#include "eventchronicstage.h"
#include "person.h"
#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace std;

// Conception happens between two people, so using this constructor seems natural.
// Also, when one of the involved persons dies before this is fired, the event is
// removed automatically.
EventTransmission::EventTransmission(Person *pPerson1, Person *pPerson2) : SimpactEvent(pPerson1, pPerson2)
{
	// is about transmission from pPerson1 to pPerson2, so no ordering according to
	// gender here
}

EventTransmission::~EventTransmission()
{
}

string EventTransmission::getDescription(double tNow) const
{
	char str[1024];

	sprintf(str, "Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
	return string(str);
}

// The dissolution event that makes this event useless involves the exact same people,
// so this function will automatically make sure that this conception event is discarded
// (this function is definitely called for those people)

bool EventTransmission::isUseless()
{
	// Transmission from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// If person2 already became HIV positive, there no sense in further transmission
	if (pPerson2->isInfected())
		return true;

	// Event is useless if the relationship between the two people is over
	if (!pPerson1->hasRelationshipWith(pPerson2))
	{
		assert(!pPerson2->hasRelationshipWith(pPerson1));
		return true;
	}

	// Make sure the two lists are consistent: if person1 has a relationship with person2, person2
	// should also have a relationship with person1
	assert(pPerson2->hasRelationshipWith(pPerson1));

	return false;
}

void EventTransmission::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	// Transmission from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// Make pPerson2 infected
	assert(pPerson1->isInfected());
	assert(!pPerson2->isInfected());
	pPerson2->setInfected(t, pPerson1, Person::Partner);

	// An event to mark the transition to the chronic stage
	EventChronicStage *pEvtChronic = new EventChronicStage(pPerson2);
	population.onNewEvent(pEvtChronic);

	// Schedule a new mortality event for person2, which will automatically be set to an aids mortality
	// since it's created after changing the infection status of the person
	EventMortality *pEvtMort = new EventMortality(pPerson2);
	population.onNewEvent(pEvtMort);

	// Schedule HIV test for pPerson2
	EventHIVTest *pEvtHIVTest = new EventHIVTest(pPerson2);
	population.onNewEvent(pEvtHIVTest);

	// Check relationships pPerson2 is in, and if the partner is not yet infected, schedule
	// a transmission event.
	int numRelations = pPerson2->getNumberOfRelationships();
	pPerson2->startRelationshipIteration();
	
	for (int i = 0 ; i < numRelations ; i++)
	{
		double formationTime = -1;
		Person *pPartner = pPerson2->getNextRelationshipPartner(formationTime);

		if (!pPartner->isInfected())
		{
			EventTransmission *pEvtTrans = new EventTransmission(pPerson2, pPartner);
			population.onNewEvent(pEvtTrans);
		}
	}

	double tDummy;
	assert(pPerson2->getNextRelationshipPartner(tDummy) == 0);

	// TODO: if a woman is infected, schedule MTCT events for children that
	//       are receiving breastfeeding

	// TODO: Survival time must be adjusted (i.e. the mortality event)
}

double EventTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO
	return dt/10;
}

double EventTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO
	return Tdiff*10;
}

