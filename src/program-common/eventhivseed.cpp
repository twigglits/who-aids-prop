#include "eventhivseed.h"
#include "eventtreatment.h"
#include "eventaidsmortality.h"
#include "eventtransmission.h"
#include "eventchronicstage.h"
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

	// First: mark a fraction of the population as infected

	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];
		assert(!pPerson->isInfected()); // No-one should be infected before seeding!

		if (pRngGen->pickRandomDouble() < initialInfectionFraction)
		{
			pPerson->setInfected(t, 0, Person::Seed);

			// introduce AIDS based mortality

			EventAIDSMortality *pAidsEvt = new EventAIDSMortality(pPerson);
			population.onNewEvent(pAidsEvt);
			
			// we're still in the acute stage and should schedule
			// an event to mark the transition to the chronic stage

			EventChronicStage *pEvtChronic = new EventChronicStage(pPerson);
			population.onNewEvent(pEvtChronic);

			// Schedule a treatment event
			
			if (EventTreatment::isTreatmentEnabled())
			{
				EventTreatment *pEvt = new EventTreatment(pPerson);
				population.onNewEvent(pEvt);
			}
		}
	}

	// We also have to check current relationships: for formed relationships, transmission
	// events may become possible
	
	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];

		// For a transmission to make sense, this person should be infected and a partner
		// should not

		if (pPerson->isInfected())
		{
			int numRelations = pPerson->getNumberOfRelationships();

			pPerson->startRelationshipIteration();
			for (int i = 0 ; i < numRelations ; i++)
			{
				double tRelation = 0;
				Person *pPartner = pPerson->getNextRelationshipPartner(tRelation);
				assert(pPartner != 0);

				if (!pPartner->isInfected())
				{
					// Ok, pPerson is infected but pPartner isn't
					// Schedule a transmission event
					
					EventTransmission *pEvtTrans = new EventTransmission(pPerson, pPartner);
					population.onNewEvent(pEvtTrans);
				}
			}

			// Just to make sure that we've covered everyone
			double tDummy = 0;
			assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
		}
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

