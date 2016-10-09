#include "eventstopbreastfeeding.h"
#include "simpactpopulation.h"
#include "person.h"
#include "eventformation.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

EventStopBreastFeeding::EventStopBreastFeeding(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventStopBreastFeeding::~EventStopBreastFeeding()
{
}

double EventStopBreastFeeding::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	Person *pPerson = getPerson(0);

	// uniform(0,2)
	double dt = pRndGen->pickRandomDouble() * 2.0;

	return dt;
}

std::string EventStopBreastFeeding::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Stop breastfeeding %s", pPerson->getName().c_str());
	return std::string(str);

}

void EventStopBreastFeeding::fire(State *pState, double t)
{
	assert(getNumberOfPersons() == 1);

	Person *pPerson = getPerson(0);
	assert(pPerson != 0);

	pPerson->stopBreastFeeding();
}

