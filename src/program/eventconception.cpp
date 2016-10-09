#include "eventconception.h"
#include "eventbirth.h"
#include "eventhivtest.h"
#include "person.h"
#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace std;

// Conception happens between two people, so using this constructor seems natural.
// Also, when one of the involved persons dies before this is fired, the event is
// removed automatically.
EventConception::EventConception(Person *pPerson1, Person *pPerson2) : SimpactEvent(pPerson1, pPerson2)
{
	// Important: the man is stored at position 0 and the woman at position 1, the code below relies on this
	assert(pPerson1->getGender() == Person::Male);
	assert(pPerson2->getGender() == Person::Female);
}

EventConception::~EventConception()
{
}

string EventConception::getDescription(double tNow) const
{
	char str[1024];

	sprintf(str, "Conception event between %s and %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
	return string(str);
}

// The dissolution event that makes this event useless involves the exact same people,
// so this function will automatically make sure that this conception event is discarded
// (this function is definitely called for those people)

bool EventConception::isUseless()
{
	Man *pMan = MAN(getPerson(0));
	Woman *pWoman = WOMAN(getPerson(1));

	if (pWoman->isPregnant())
	{
		//cerr << "Conception with " << pPerson1->getName() << " is useless because of pregnancy of " << pPerson2->getName() << endl;
		return true;
	}

	// Event is useless if the relationship between the two people is over

	if (!pMan->hasRelationshipWith(pWoman))
	{
		assert(!pWoman->hasRelationshipWith(pMan));
		//cerr << "Conception with " << pPerson1->getName() << " is useless because relationship with " << pPerson2->getName() << " is over" << endl;
		return true;
	}

	// Make sure the two lists are consistent: if person1 has a relationship with person2, person2
	// should also have a relationship with person1
	assert(pWoman->hasRelationshipWith(pMan));

	return false;
}

void EventConception::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);

	Man *pMan = MAN(getPerson(0));
	Woman *pWoman = WOMAN(getPerson(1));
	
	assert(!pWoman->isPregnant());
	pWoman->setPregnant(true);

	EventBirth *pEvtBirth = new EventBirth(pWoman);
	
	// Note: also store who's the father in this event (we can't use the constructor because
	//       the system will think two people are needed for the birth event, causing it to
	//       be deleted if the father dies for example)
	pEvtBirth->setFather(pMan);

	population.onNewEvent(pEvtBirth);

	// Also schedule HIV test event for pregnant women
	EventHIVTest *pEvtHIVTest = new EventHIVTest(pWoman);
	population.onNewEvent(pEvtHIVTest);
}

double EventConception::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO
	return dt/300.0;
}

double EventConception::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO
	return Tdiff*300.0;
}

