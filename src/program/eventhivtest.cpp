#include "eventhivtest.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

using namespace std;

EventHIVTest::EventHIVTest(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventHIVTest::~EventHIVTest()
{
}

bool EventHIVTest::isUseless()
{
	Person *pPerson = getPerson(0);

	if (!pPerson->isInfected())
	{
		// TODO: for the current setup this only happens for a pregnant woman

		assert(pPerson->isWoman());

		Woman *pWoman = WOMAN(pPerson);

		// When the woman has given birth, this event becomes useless
		// TODO: is this correct?
		if (!pWoman->isPregnant())
		{
			cerr << "    HIV test became useless for " << pWoman->getName() << endl;
			return true;
		}
	}
	return false;
}

double EventHIVTest::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	double dt = 0.5; // TODO: half a year fixed? Or add some randomness?

	// I'm adding a very small amount of randomness (order of a day) to avoid events firing at the same
	// time (is not a problem for the algorithm, but may cause a different order of events between the
	// advanced and basic algorithms)
	
	double r = pRndGen->pickRandomDouble()*2.0-1.0; // between -1 and 1

	dt += r * 1.0/365.0;

	return dt;
}

std::string EventHIVTest::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "HIV test event for %s", pPerson->getName().c_str());
	return std::string(str);
}

void EventHIVTest::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	// Schedule a new test
	EventHIVTest *pEvtHIVTest = new EventHIVTest(pPerson);
	population.onNewEvent(pEvtHIVTest);

	// TODO: check CD4 count and VL and start treatment if necessary
	// TODO: can this be done here? Or do we need to schedule a treatment event?
	
	// TODO: schedule test events for partners/children
}

