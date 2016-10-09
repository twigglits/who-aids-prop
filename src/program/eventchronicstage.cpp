#include "eventchronicstage.h"
#include "simpactpopulation.h"
#include "person.h"
#include "eventformation.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

EventChronicStage::EventChronicStage(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventChronicStage::~EventChronicStage()
{
}

double EventChronicStage::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);
	assert(pPerson->isInfected());

	double tEvt = pPerson->getInfectionTime() + population.getAcuteStageTime();
	double dt = tEvt - population.getTime();
	
	assert(dt >= 0); // should not be in the past!

	return dt;
}

std::string EventChronicStage::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Chronic infection stage for %s", pPerson->getName().c_str());
	return std::string(str);

}

void EventChronicStage::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);

	Person *pPerson = getPerson(0);
	assert(pPerson != 0);
	assert(pPerson->inAcuteStage());

	pPerson->setInChronicStage();
}

