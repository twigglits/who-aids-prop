#include "eventdebut.h"
#include "eventformation.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
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

	// No relationships will be scheduled if the person is already in the final AIDS stage
	if (pPerson->getInfectionStage() != Person::AIDSFinal)
		population.initializeFormationEvents(pPerson);
}

double EventDebut::m_debutAge = -1;

void EventDebut::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (!config.getKeyValue("debut.debutage", m_debutAge, 0, 100))
		abortWithMessage(config.getErrorString());
}

void EventDebut::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("debut.debutage", m_debutAge))
		abortWithMessage(config.getErrorString());
}

ConfigFunctions debutConfigFunctions(EventDebut::processConfig, EventDebut::obtainConfig, "EventDebut");

JSONConfig debutJSONConfig(R"JSON(
        "EventDebut": { 
            "depends": null, 
            "params" : [ [ "debut.debutage", 15] ],
            "info": [
                "Age at which a person becomes sexually active and can form",
                "relationships"
            ]
        })JSON");

