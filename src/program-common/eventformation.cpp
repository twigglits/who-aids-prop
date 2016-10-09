#include "eventformation.h"
#include "eventdissolution.h"
#include "eventtransmission.h"
#include "eventdebut.h"
#include "simpactpopulation.h"
#include "simpactevent.h"
#include "evthazardformationsimple.h"
#include "evthazardformationagegap.h"
#include "eventconception.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

EventFormation::EventFormation(Person *pPerson1, Person *pPerson2, double lastDissTime) : SimpactEvent(pPerson1, pPerson2)
{
	m_lastDissolutionTime = lastDissTime;

	// Just a check because in the hazard we'll be interpreting a1 as a factor for the
	// men and a2 as a factor for the women
	assert(pPerson1->isMan());
	assert(pPerson2->isWoman());

	assert(pPerson1->isSexuallyActive());
	assert(pPerson2->isSexuallyActive());

	// New formation events must not be scheduled if one of the persons is in the
	// final AIDS stage
	assert(pPerson1->getInfectionStage() != Person::AIDSFinal);
	assert(pPerson2->getInfectionStage() != Person::AIDSFinal);
}

EventFormation::~EventFormation()
{
}

std::string EventFormation::getDescription(double tNow) const
{
	char str[1024];

	sprintf(str, "Formation between %s and %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
	return std::string(str);
}

void EventFormation::writeLogs(double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "formation", tNow, pPerson1, pPerson2);
}

bool EventFormation::isUseless()
{
	// Formation event becomes useless if one of the people is in the final AIDS
	// stage
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	
	if (pPerson1->getInfectionStage() == Person::AIDSFinal || pPerson2->getInfectionStage() == Person::AIDSFinal)
		return true;

	return false;
}

void EventFormation::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	pPerson1->addRelationship(pPerson2, t);
	pPerson2->addRelationship(pPerson1, t);

	// Need to add a dissolution event

	EventDissolution *pDissEvent = new EventDissolution(pPerson1, pPerson2, t);
	population.onNewEvent(pDissEvent);

	Woman *pWoman = WOMAN(pPerson2);

	if (!pWoman->isPregnant())
	{
		EventConception *pEvtConception = new EventConception(pPerson1, pPerson2, t); // should be: man, woman, formation time
		population.onNewEvent(pEvtConception);
	}

	/*
	if (m_lastDissolutionTime >= 0)
	{
		std::cerr << "New formation between " << pPerson1->getName() << " and " << pPerson2->getName() << " after " << (t-m_lastDissolutionTime) << " years" << std::endl;
	}
	*/

	// If one of the partners is infected (but not both), schedule a
	// transmission event

	if (pPerson1->isInfected())
	{
		if (!pPerson2->isInfected())
		{
			EventTransmission *pEvtTrans = new EventTransmission(pPerson1, pPerson2);
			population.onNewEvent(pEvtTrans);
		}
	}
	else // pPerson1 not infected
	{
		if (pPerson2->isInfected())
		{
			EventTransmission *pEvtTrans = new EventTransmission(pPerson2, pPerson1);
			population.onNewEvent(pEvtTrans);
		} 
	}
}

double EventFormation::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	assert(m_pHazard != 0);

	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	return m_pHazard->calculateInternalTimeInterval(population, *this, t0, dt);
}

double EventFormation::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	assert(m_pHazard != 0);

	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	return m_pHazard->solveForRealTimeInterval(population, *this, Tdiff, t0);
}

EvtHazard *EventFormation::m_pHazard = 0;

void EventFormation::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	string hazardType;

	if (!config.getKeyValue("formation.hazard.type", hazardType))
		abortWithMessage(config.getErrorString());

	if (m_pHazard)
	{
		delete m_pHazard;
		m_pHazard = 0;
	}

	if (hazardType == "simple")
		m_pHazard = EvtHazardFormationSimple::processConfig(config);
	else if (hazardType == "agegap")
		m_pHazard = EvtHazardFormationAgeGap::processConfig(config);
	else
		abortWithMessage("Unknown formation.hazard.type value: " + hazardType);
}

void EventFormation::obtainConfig(ConfigWriter &config)
{
	if (!m_pHazard)
		abortWithMessage("EventFormation::obtainConfig: m_pHazard is null");

	m_pHazard->obtainConfig(config);
}

ConfigFunctions formationConfigFunctions(EventFormation::processConfig, EventFormation::obtainConfig, "EventFormation");

JSONConfig formationTypesJSONConfig(R"JSON(
        "EventFormationTypes": { 
            "depends": null,
            "params": [ ["formation.hazard.type", "agegap", [ "simple", "agegap" ] ] ],
            "info": null 
        })JSON");

