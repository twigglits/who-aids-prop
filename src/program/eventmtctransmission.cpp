#include "eventmtctransmission.h"
#include "eventhivtest.h"
#include "eventmortality.h"
#include "eventchronicstage.h"
#include "person.h"
#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace std;

// Transmission becomes impossible when one of the two dies, so this 2-person
// event seem a good choice
EventMTCTransmission::EventMTCTransmission(Person *pMother, Person *pChild) : SimpactEvent(pMother, pChild)
{
}

EventMTCTransmission::~EventMTCTransmission()
{
}

string EventMTCTransmission::getDescription(double tNow) const
{
	char str[1024];

	sprintf(str, "MTC transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
	return string(str);
}

// The dissolution event that makes this event useless involves the exact same people,
// so this function will automatically make sure that this conception event is discarded
// (this function is definitely called for those people)

bool EventMTCTransmission::isUseless()
{
	// Transmission from mother to child
	Woman *pMother = WOMAN(getPerson(0));
	Person *pChild = getPerson(1);

	assert(!pChild->isInfected()); // Child should not yet be infected by some other means
	assert(pChild->getMother() == pMother);	// Make sure the child and mother are consistent:
	assert(pMother->hasChild(pChild));

	// Check if still breastfeeding
	if (!pChild->isBreastFeeding())
		return true;
	
	return false;
}

void EventMTCTransmission::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);

	// Transmission from mother to child
	Woman *pMother = WOMAN(getPerson(0));
	Person *pChild = getPerson(1);

	// Make pChild infected
	assert(pMother->isInfected());
	assert(!pChild->isInfected());

	pChild->setInfected(t, pMother, Person::Mother);

	// Schedule a new mortality event for child, which will automatically be set to an aids mortality
	EventMortality *pEvtMort = new EventMortality(pChild);
	population.onNewEvent(pEvtMort);

	// Schedule HIV test
	EventHIVTest *pEvtHIVTest = new EventHIVTest(pChild);
	population.onNewEvent(pEvtHIVTest);

	// Shedule transition to the chronic stage
	EventChronicStage *pEvtChronic = new EventChronicStage(pChild);
	population.onNewEvent(pEvtChronic);
}

double EventMTCTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO
	return dt/10;
}

double EventMTCTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO
	return Tdiff*10;
}

