#include "eventhivseed.h"
#include "eventaidsmortality.h"
#include "eventtransmission.h"
#include "eventchronicstage.h"
#include "eventtest.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

EventHIVSeed::EventHIVSeed()
{
}

EventHIVSeed::~EventHIVSeed()
{
}

double EventHIVSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);

	double dt = m_seedTime - population.getTime();
	assert(m_seedTime >= 0);
	assert(dt >= 0);

	return dt;
}

std::string EventHIVSeed::getDescription(double tNow) const
{
	return std::string("HIV seeding");
}

void EventHIVSeed::writeLogs(double tNow) const
{
	writeEventLogStart(true, "HIV seeding", tNow, 0, 0);
}

void EventHIVSeed::fire(State *pState, double t)
{
	// Check that this event is only carried out once
	assert(!m_seeded);
	m_seeded = true;

	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person **ppPeople = population.getAllPeople();
	int numPeople = population.getNumberOfPeople();
	GslRandomNumberGenerator *pRngGen = population.getRandomNumberGenerator();

	double initialInfectionFraction = m_seedFraction;

	assert(initialInfectionFraction >= 0 && initialInfectionFraction <= 1.0);

	// Mark a fraction of the population as infected

	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];
		assert(!pPerson->isInfected()); // No-one should be infected before seeding!

		if (pRngGen->pickRandomDouble() < initialInfectionFraction)
			EventTransmission::infectPerson(population, 0, pPerson, t); // 0 means seed
	}
}

double EventHIVSeed::m_seedTime = -1;
double EventHIVSeed::m_seedFraction = -1;
bool EventHIVSeed::m_seeded = false;

void EventHIVSeed::processConfig(ConfigSettings &config)
{
	if (!config.getKeyValue("hivseed.time", m_seedTime) ||
	    !config.getKeyValue("hivseed.fraction", m_seedFraction, 0, 1) )
		abortWithMessage(config.getErrorString());
}

void EventHIVSeed::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("hivseed.time", m_seedTime) ||
	    !config.addKey("hivseed.fraction", m_seedFraction) )
		abortWithMessage(config.getErrorString());
}

