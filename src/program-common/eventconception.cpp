#include "eventconception.h"
#include "configdistributionhelper.h"
#include "probabilitydistribution.h"
#include "eventbirth.h"
#include <assert.h>

using namespace std;

EventConception::EventConception(Person *pPerson1, Person *pPerson2, double relationshipFormationTime) : SimpactEvent(pPerson1, pPerson2)
{
	assert(pPerson1 && pPerson2);
	assert(pPerson1->isMan() && pPerson2->isWoman());

	assert(m_pWSFProbDist);
	m_WSF = m_pWSFProbDist->pickNumber();
	m_relationshipFormationTime = relationshipFormationTime;
}

EventConception::~EventConception()
{
}

string EventConception::getDescription(double tNow) const
{
	return "Conception";
}

void EventConception::writeLogs(double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "conception", tNow, pPerson1, pPerson2);
}

void EventConception::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);

	Man *pMan = MAN(getPerson(0));
	Woman *pWoman = WOMAN(getPerson(1));
	
	assert(!pWoman->isPregnant());
	pWoman->setPregnant(true);

	EventBirth *pEvtBirth = new EventBirth(pWoman);
	// Note: also store who's the father in this event (we can't use the constructor because
	//       the system will think two people are needed for the birth event, causing it to
	//       be deleted if the father dies for example)
	pEvtBirth->setFather(pMan);

	population.onNewEvent(pEvtBirth);
}

double EventConception::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	double tr = m_relationshipFormationTime;
	double tMax = getTMax(pPerson1, pPerson2);

	HazardFunctionConception h0(pPerson1, pPerson2, m_WSF, tr);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventConception::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	double tr = m_relationshipFormationTime;
	double tMax = getTMax(pPerson1, pPerson2);

	HazardFunctionConception h0(pPerson1, pPerson2, m_WSF, tr);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

bool EventConception::isUseless()
{
	Man *pMan = MAN(getPerson(0));
	Woman *pWoman = WOMAN(getPerson(1));

	// Is the woman is already pregnant, new conceptions are no longer possible
	if (pWoman->isPregnant())
		return true;

	// If the relationship between the two people is over, this event is cancelled
	// as well
	if (!pWoman->hasRelationshipWith(pMan))
	{
		assert(!pMan->hasRelationshipWith(pWoman)); // consistency check
		return true;
	}

	assert(pMan->hasRelationshipWith(pWoman)); // consistency check
	return false;
}

double EventConception::getTMax(const Person *pPerson1, const Person *pPerson2)
{
	assert(pPerson1 != 0 && pPerson2 != 0);

	double tb1 = pPerson1->getDateOfBirth();
	double tb2 = pPerson2->getDateOfBirth();

	double tMax = tb1;

	if (tb2 < tMax)
		tMax = tb2;

	assert(m_tMax > 0);
	tMax += m_tMax;
	return tMax;
}

double EventConception::HazardFunctionConception::m_alphaBase = 0;
double EventConception::HazardFunctionConception::m_alphaAgeMan = 0;
double EventConception::HazardFunctionConception::m_alphaAgeWoman = 0;
double EventConception::HazardFunctionConception::m_alphaWSF = 0;
double EventConception::HazardFunctionConception::m_beta = 0;

double EventConception::m_tMax = 0;
ProbabilityDistribution *EventConception::m_pWSFProbDist = 0;

void EventConception::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (m_pWSFProbDist)
	{
		delete m_pWSFProbDist;
		m_pWSFProbDist = 0;
	}

	if (!config.getKeyValue("conception.alpha_base", HazardFunctionConception::m_alphaBase) ||
	    !config.getKeyValue("conception.alpha_ageman", HazardFunctionConception::m_alphaAgeMan) ||
	    !config.getKeyValue("conception.alpha_agewoman", HazardFunctionConception::m_alphaAgeWoman) ||
	    !config.getKeyValue("conception.alpha_wsf", HazardFunctionConception::m_alphaWSF) ||
	    !config.getKeyValue("conception.beta", HazardFunctionConception::m_beta) ||
	    !config.getKeyValue("conception.t_max", m_tMax, 0)
		)
		abortWithMessage(config.getErrorString());

	m_pWSFProbDist = getDistributionFromConfig(config, pRndGen, "conception.wsf");
}

void EventConception::obtainConfig(ConfigWriter &config)
{
	addDistributionToConfig(m_pWSFProbDist, config, "conception.wsf");

	if (!config.addKey("conception.alpha_base", HazardFunctionConception::m_alphaBase) ||
	    !config.addKey("conception.alpha_ageman", HazardFunctionConception::m_alphaAgeMan) ||
	    !config.addKey("conception.alpha_agewoman", HazardFunctionConception::m_alphaAgeWoman) ||
	    !config.addKey("conception.alpha_wsf", HazardFunctionConception::m_alphaWSF) ||
	    !config.addKey("conception.beta", HazardFunctionConception::m_beta) ||
	    !config.addKey("conception.t_max", m_tMax)
		)
		abortWithMessage(config.getErrorString());
}

EventConception::HazardFunctionConception::HazardFunctionConception(const Person *pMan, const Person *pWoman, 
		                                                    double WSF, double tRef)
{
	assert(pMan && pWoman);
	assert(pMan->isMan() && pWoman->isWoman());

	double tBMan = pMan->getDateOfBirth();
	double tBWoman = pWoman->getDateOfBirth();

	double A = m_alphaBase - m_alphaAgeMan*tBMan - m_alphaAgeWoman*tBWoman + m_alphaWSF*WSF - m_beta*tRef;
	double B = m_alphaAgeMan + m_alphaAgeWoman + m_beta;

	// set the parameters of exp(A+B*t) in the base class
	setAB(A, B);
}

EventConception::HazardFunctionConception::~HazardFunctionConception()
{
}

