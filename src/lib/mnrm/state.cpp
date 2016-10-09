#include <omp.h>
#include "state.h"
#include "eventbase.h"
#include "gslrandomnumbergenerator.h"
#include "debugwarning.h"
#include <assert.h>
#include <iostream>
#include <limits>

State::State(GslRandomNumberGenerator *pRng)
{
	m_pRndGen = pRng;
	m_time = 0;

#ifdef STATE_SHOW_EVENTS
	DEBUGWARNING("debug code to list events is enabled")
#endif // STATE_SHOW_EVENTS
}

State::~State()
{
}

bool State::evolve(double &tMax, int64_t &maxEvents, double startTime, bool initEvents)
{
	m_time = startTime;

	if (initEvents)
	{
		if (!initEventTimes())
		{
			setErrorString("Requested event time initialization, but unable to do this: " + getErrorString());
			return false;
		}
	}

	bool done = false;
	int64_t eventCount = 0;

	while (!done)
	{
		// Ask for the next scheduled event and for the time until it takes place
		double dtMin = -1;
		EventBase *pNextScheduledEvent = getNextScheduledEvent(dtMin);

		if (pNextScheduledEvent == 0)
		{
			setErrorString("No next scheduled event found: " + getErrorString());
			tMax = m_time;
			maxEvents = eventCount;
			return false;
		}

		//std::cerr << "dtMin = " << dtMin << std::endl;
		assert(dtMin >= 0);

		// Advance the times of all events but the next scheduled one
		advanceEventTimes(pNextScheduledEvent, dtMin);
	
		// ok, advance time and fire the event, which may adjust the current state
		// and generate a new internal time difference

		m_time += dtMin;

		onAboutToFire(pNextScheduledEvent);
		pNextScheduledEvent->fire(this, m_time);

		// If the event is still being used (the default) we'll need a new random number
		if (!pNextScheduledEvent->willBeRemoved())
			pNextScheduledEvent->generateNewInternalTimeDifference(m_pRndGen, this);

		eventCount++;

		if (m_time > tMax || (maxEvents > 0 && eventCount >= maxEvents))
			done = true;

		onFiredEvent(pNextScheduledEvent);
		onAlgorithmLoop(done);
	}

	// inform the caller of the current time and number of events
	tMax = m_time;
	maxEvents = eventCount;
	
	return true;
}

bool State::initEventTimes() const
{
	setErrorString("State::initEventTimes: not implemented in base class");
	return false;
}

EventBase *State::getNextScheduledEvent(double &dt)
{
	setErrorString("State::getNextScheduledEvent: not implemented in base class");
	return 0;
}

void State::advanceEventTimes(EventBase *pScheduledEvent, double dtMin)
{
	// Not implemented in base class
}

