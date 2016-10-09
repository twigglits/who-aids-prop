#include "eventbirth.h"
#include "eventdebut.h"
#include "eventmortality.h"
#include "eventconception.h"
#include "eventmtctransmission.h"
#include "eventstopbreastfeeding.h"
#include "eventchronicstage.h"
#include "simpactpopulation.h"
#include "person.h"
#include "gslrandomnumbergenerator.h"
#include <stdio.h>
#include <iostream>

// The event should not be affected if the father dies, so it's a one person event
// but we'll store the father using 'setFather' to keep track of him
EventBirth::EventBirth(Person *pPerson) : SimpactEvent(pPerson)
{
	m_pFather = 0;
}

EventBirth::~EventBirth()
{
}

void EventBirth::setFather(Person *pFather)
{
	m_pFather = MAN(pFather);
}

double EventBirth::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	double dt = 268.0/365.0; // 268 days // TODO: make this configurable? // TODO: add something random?

	return dt;
}

std::string EventBirth::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Birth event for %s", pPerson->getName().c_str());
	return std::string(str);
}

void EventBirth::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);

	Woman *pMother = WOMAN(getPerson(0));
	assert(pMother->isPregnant());

	// Create the new person, set father and mother and add him/her to the population
	double boyGirlRatio = 1.0/2.01;
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pChild = 0;
	
	if (pRndGen->pickRandomDouble() < boyGirlRatio)
		pChild = new Man(t);
	else
		pChild = new Woman(t);

	assert(m_pFather != 0);
	pChild->setFather(m_pFather);
	pChild->setMother(pMother);

	m_pFather->addChild(pChild); // TODO: this is also done if the father is deceased, can be useful I think
	pMother->addChild(pChild);

	population.addNewPerson(pChild);
	std::cerr << "\t\tNew child born: " << pChild->getName() << std::endl;

	// Schedule MTC transmission event if the mother is infected, or
	
	if (pMother->isInfected())
	{
		if (0) // TODO TODO TODO: what criterion should be used here?
		{
			// Child is immediately infected
			pChild->setInfected(t, pMother, Person::Mother);
			
			// the mortality event is generated below and will automatically be of the
			// right type

			// Schedule an event for the transition to the chronic stage
			EventChronicStage *pEvtChronic = new EventChronicStage(pChild);
			population.onNewEvent(pEvtChronic);
		}
		else // not infected immediately
		{
			EventMTCTransmission *pEvtMTCTrans = new EventMTCTransmission(pMother, pChild);
			population.onNewEvent(pEvtMTCTrans);
		}
	}

	// TODO: only do breastfeeding stuff in case of a mtc transmission event?
	//       (child is already infected otherwise)
	// Schedule breastfeeding stop time
	pChild->setBreastFeeding();

	EventStopBreastFeeding *pEvtStopBF = new EventStopBreastFeeding(pChild); // it's the child which is being fed
	population.onNewEvent(pEvtStopBF);

	// Create mortality and debut events for the child and register them in the system
	// Since the infection status has already been set, this can be both a normal and an
	// aids-mortality
	EventMortality *pEvtMort = new EventMortality(pChild);
	population.onNewEvent(pEvtMort);

	EventDebut *pEvtDebut = new EventDebut(pChild);
	population.onNewEvent(pEvtDebut);

	// After the birth the pregnancy is over
	pMother->setPregnant(false);
	
	// Schedule conception events for current relationships the mother is in
	int numRelations = pMother->getNumberOfRelationships();
	pMother->startRelationshipIteration();
	
	for (int i = 0 ; i < numRelations ; i++)
	{
		double formationTime = -1;
		Person *pPartner = pMother->getNextRelationshipPartner(formationTime);

		assert(pPartner->getGender() == Person::Male);

		EventConception *pEvtCon = new EventConception(pPartner, pMother);
		population.onNewEvent(pEvtCon);
	}

	double tDummy;
	assert(pMother->getNextRelationshipPartner(tDummy) == 0);
}

