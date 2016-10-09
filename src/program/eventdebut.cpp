#include "eventdebut.h"
#include "eventformation.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

EventDebut::EventDebut(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventDebut::~EventDebut()
{
}

double EventDebut::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);

	assert(m_debutAge > 0);

	double tEvt = pPerson->getDateOfBirth() + m_debutAge;
	double dt = tEvt - population.getTime();

	return dt;
}

std::string EventDebut::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Debut of %s", pPerson->getName().c_str());
	return std::string(str);
}

void EventDebut::writeLogs(double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "debut", tNow, pPerson, 0);
}

void EventDebut::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);

	Person *pPerson = getPerson(0);
	assert(pPerson != 0);

	pPerson->setSexuallyActive(t);

	// code should reduce to the old version where everyone can have a relationship with
	// everyone in case eyeCapsFraction == 1
	double eyeCapsFraction = population.getEyeCapsFraction();
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();

	assert(eyeCapsFraction >= 0 && eyeCapsFraction <= 1.0);

	// No relationships will be scheduled if the person is already in the final AIDS stage
	if (pPerson->getInfectionStage() != Person::AIDSFinal)
	{
		// TODO: this is not correct if we're using a limited set of interests
		if (pPerson->getGender() == Person::Male)
		{
			Man *pMan = MAN(pPerson);
			Woman **ppWomen = population.getWomen();
			int numWomen = population.getNumberOfWomen();

			for (int i = 0 ; i < numWomen ; i++)
			{
				Woman *pWoman = ppWomen[i];
				
				if (pWoman->isSexuallyActive() && pWoman->getInfectionStage() != Person::AIDSFinal)
				{
					// Don't necessarily schedule all possibilities
					if (eyeCapsFraction >= 1.0 || pRndGen->pickRandomDouble() < eyeCapsFraction)
					{
						EventFormation *pEvt = new EventFormation(pMan, pWoman, -1);
						population.onNewEvent(pEvt);
					}
				}
			}
		}
		else // Female
		{
			Woman *pWoman = WOMAN(pPerson);
			Man **ppMen = population.getMen();
			int numMen = population.getNumberOfMen();

			for (int i = 0 ; i < numMen ; i++)
			{
				Man *pMan = ppMen[i];

				if (pMan->isSexuallyActive() && pMan->getInfectionStage() != Person::AIDSFinal)
				{
					// Don't necessarily schedule all possibilities
					if (eyeCapsFraction >= 1.0 || pRndGen->pickRandomDouble() < eyeCapsFraction)
					{
						EventFormation *pEvt = new EventFormation(pMan, pWoman, -1);
						population.onNewEvent(pEvt);
					}
				}
			}
		}
	}
}

double EventDebut::m_debutAge = -1;

void EventDebut::processConfig(ConfigSettings &config)
{
	if (!config.getKeyValue("debut.debutage", m_debutAge, 0, 100))
		abortWithMessage(config.getErrorString());
}

void EventDebut::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("debut.debutage", m_debutAge))
		abortWithMessage(config.getErrorString());
}
