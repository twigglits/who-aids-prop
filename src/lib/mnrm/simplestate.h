#ifndef SIMPLESTATE_H

#define SIMPLESTATE_H

/**
 * \file simplestate.h
 */

#include "state.h"
#include <vector>

class GslRandomNumberGenerator;
class EventBase;

/** A very naive implementation of the necessary functions from the State
 *  class.
 *
 *  In this implementation, all event times are recalculated each time an
 *  event has fired. This is slow, but a good reference implemenation to
 *  check an optimized version against. To create a simulation with this
 *  version of the algorithm, you need to provide implementations for the
 *  SimpleState::getCurrentEvents and SimpleState::onFiredEvent functions.
 *  The first function must return a list of all events in the system,
 *  the second one can modify the list, for example removing the fired
 *  event if no longer necessary.
 *
 *  The code that implements State::initEventTimes looks as follow:
 *  @code
 * const std::vector<EventBase *> &events = getCurrentEvents();
 *
 * for (int i = 0 ; i < events.size() ; i++)
 *	events[i]->generateNewInternalTimeDifference(pRndGen, this);
 *  @endcode
 *
 *  The code for the State::getNextScheduledEvent implementation looks
 *  as follows:
 *  @code
 * const std::vector<EventBase *> &events = getCurrentEvents();
 * double curTime = getTime();
 * double dtMin = MAX_DOUBLE;
 * int eventPos = -1;
 *
 * for (int i = 0 ; i < events.size() ; i++)
 * {
 *	// This function calcutates the real-world interval that corresponds
 *	// to the stored internal time interval. 
 * 	double dt = events[i]->solveForRealTimeInterval(this, curTime);
 *
 * 	if (dt < dtMin)
 *	{
 *		dtMin = dt;
 *		eventPos = i;
 *	}
 * }
 *  @endcode
 *
 *  Here, the key calculation is the \c solveForRealTimeInterval one, which
 *  solves for \f$ dt \f$ in the integral
 *  \f[ \Delta T = \int_{\rm curTime }^{{\rm curTime} + dt} h(X({\rm curTime}),s) ds \f]
 *  where \f$ \Delta T \f$ is the current (remaining) internal time interval for an event.
 *
 * Finally, the State::advanceEventTimes implementation is the following:
 *  @code
 * double t1 = curTime + dtMin;
 *
 * for (int i = 0 ; i < events.size() ; i++)
 * {
 * 	if (i != eventPos)
 * 		// This function subtracts from the internal time the
 * 		// amount that corresponds to advancing the real world
 *		// time to t1
 *		events[i]->subtractInternalTimeInterval(this, t1);
 * }
 *  @endcode
 *  Here, the internal time \f$ \Delta T \f$ for an event is replaced by
 *  \f[ \Delta T - \int_{t^c}^{t_1} h(X(t^c),s) ds \f]
 *  where \f$ t_1 = {\rm curTime} + {\rm dtMin} \f$ and \f$t_c\f$ is the time at
 *  which the mapping between internal time interval and real world fire time
 *  was last calculated (see \ref index "main page").
 *
 *  Note that if you still need to implement the State::onFiredEvent function
 *  yourself, you must call the corresponding implementation of SimpleState
 *  as well. Otherwise the algorithm will not work properly anymore.
 *
 */
class SimpleState : public State
{
public:
	/** Constructor of this class, specifying whether a parallel version should
	 *  be used to speed things up a bit, and specifying the random number
	 *  generator. */
	SimpleState(bool parallel, GslRandomNumberGenerator *pRng);
	~SimpleState();
protected:
	/** This function should return the list of events that are currently scheduled. */
	virtual const std::vector<EventBase *> &getCurrentEvents() const 				{ return m_dummyEventList; }

	/** This function is called after firing the event \c pEvt, stored at position
	 *  \c position in the list that was returned by SimpleState::getCurrentEvents,
	 *  and this function should remove at least that event from the list. */
	virtual void onFiredEvent(EventBase *pEvt, int position)					{ }
private:
	bool initEventTimes() const;
	EventBase *getNextScheduledEvent(double &dt);
	void advanceEventTimes(EventBase *pScheduledEvent, double dt);
	void onFiredEvent(EventBase *pEvt);

#ifdef STATE_SHOW_EVENTS
	virtual void showEvents() { } // For debugging
#endif // STATE_SHOW_EVENTS

	std::vector<EventBase *> m_dummyEventList;
	const std::vector<EventBase *> *m_pTmpEventList;

	int m_eventPos;

	// for parallel version
	std::vector<double> m_dtMinValues;
	std::vector<int> m_minPosValues;
	bool m_parallel;
};

#endif // SIMPLESTATE_H

