#include "eventbase.h"
#include "gslrandomnumbergenerator.h"
#include "debugwarning.h"
#include <cmath>

EventBase::EventBase()
{
	m_Tdiff = -10000000.0; // should trigger an assertion in debug mode
	m_tLastCalc = -1;
	m_tEvent = -2;
	m_willBeRemoved = false;

#ifdef EVENTBASE_CHECK_INVERSE
	DEBUGWARNING("debug code to check solveForRealTimeInterval <-> calculateInternalTimeInterval compatibility is enabled")
#endif // EVENTBASE_CHECK_INVERSE
}

EventBase::~EventBase()
{
}

void EventBase::generateNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	m_Tdiff = getNewInternalTimeDifference(pRndGen, pState);
	setNeedEventTimeCalculation();
}

double EventBase::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	double r = pRndGen->pickRandomDouble();

//	TODO: is there a reason to prefer the first one over the second?
//	double dT = std::log(1.0/r);
	double dT = -std::log(r);

	return dT;
}

// In this default implementation there's no difference between internal time and real world time,
// meaning that for a fixed time event, you'd just need to implement the getNewInternalTimeDifference
// to generate a real world time interval
double EventBase::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	return dt;
}

double EventBase::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	return Tdiff;
}

void EventBase::fire(State *pState, double t)
{
	// test implementation: nothing happens
}
