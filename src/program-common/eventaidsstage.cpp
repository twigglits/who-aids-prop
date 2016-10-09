#include "eventaidsstage.h"
#include "eventaidsmortality.h"

using namespace std;

EventAIDSStage::EventAIDSStage(Person *pPerson, bool final) : SimpactEvent(pPerson)
{
	m_finalStage = final;
}

EventAIDSStage::~EventAIDSStage()
{
}

string EventAIDSStage::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	if (m_finalStage)
		sprintf(str, "Final AIDS stage of %s", pPerson->getName().c_str());
	else
		sprintf(str, "AIDS stage of %s", pPerson->getName().c_str());

	return string(str);
}

void EventAIDSStage::writeLogs(double tNow) const
{
	Person *pPerson = getPerson(0);
	string name;

	if (m_finalStage)
		name = "finalaidsstage";
	else
		name = "aidsstage";

	writeEventLogStart(true, name, tNow, pPerson, 0);
}

void EventAIDSStage::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	if (m_finalStage)
	{
		pPerson->setInFinalAIDSStage();
	}
	else
	{
		pPerson->setInAIDSStage();

		// If we're not yet in the final stage, we need to schedule an event again
		EventAIDSStage *pEvt = new EventAIDSStage(pPerson, true);
		population.onNewEvent(pEvt);
	}
}

double EventAIDSStage::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	double currentTime = pState->getTime();
	double newStageTime = getNewStageTime(currentTime);
	assert(newStageTime > currentTime);

	m_eventHelper.setFireTime(newStageTime);
	return m_eventHelper.getNewInternalTimeDifference(pRndGen, pState);
}

double EventAIDSStage::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	checkFireTime(t0);
	return m_eventHelper.calculateInternalTimeInterval(pState, t0, dt, this);
}

double EventAIDSStage::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	checkFireTime(t0);
	return m_eventHelper.solveForRealTimeInterval(pState, Tdiff, t0, this);
}

void EventAIDSStage::checkFireTime(double t0)
{
	double newStageTime = getNewStageTime(t0);
	assert(newStageTime > t0); // TODO: a harder check here? Also in release mode?

	if (m_eventHelper.getFireTime() != newStageTime)
		m_eventHelper.setFireTime(newStageTime);
}

double EventAIDSStage::getNewStageTime(double currentTime) const
{
	const Person *pPerson = getPerson(0);

	double expectedTimeOfDeath = pPerson->getAIDSMortalityTime();
	double newStageTime = expectedTimeOfDeath;

	if (m_finalStage)
	{
		assert(pPerson->getInfectionStage() == Person::AIDS);
		newStageTime -= m_relativeFinalTime;
	}
	else
	{
		assert(pPerson->getInfectionStage() == Person::Chronic);
		newStageTime -= m_relativeStartTime;
	}
	
	// TODO: What's  a good approach in this case? for now, we'll advance to the
	//       stage immediately
	if (newStageTime <= currentTime)
	{
		newStageTime = currentTime + 1e-8;
//		cerr << "Warning: advancing faster to new AIDS stage for person " << pPerson->getName() << endl;
	}

	return newStageTime;
}

double EventAIDSStage::m_relativeStartTime = -1;
double EventAIDSStage::m_relativeFinalTime = -1;

void EventAIDSStage::processConfig(ConfigSettings &config)
{
	if (!config.getKeyValue("aidsstage.final", m_relativeFinalTime, 0) ||
	    !config.getKeyValue("aidsstage.start", m_relativeStartTime, m_relativeFinalTime) )
		abortWithMessage(config.getErrorString());
}

void EventAIDSStage::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("aidsstage.final", m_relativeFinalTime) ||
	    !config.addKey("aidsstage.start", m_relativeStartTime) )
		abortWithMessage(config.getErrorString());
}

