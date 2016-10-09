#ifndef DISABLEOPENMP
#include <omp.h>
#endif // !DISABLEOPENMP

#include "simplestate.h"
#include "eventbase.h"
#include "gslrandomnumbergenerator.h"
#include "util.h"
#include <assert.h>
#include <iostream>
#include <limits>

SimpleState::SimpleState(bool parallel, GslRandomNumberGenerator *pRng) : State(pRng)
{
#ifdef DISABLEOPENMP
	if (parallel)
		abortWithMessage("Parallel version requested but OpenMP was not available when creating the program");
#endif // DISABLEOPENMP
	m_parallel = parallel;

	m_eventPos = -1;
	m_pTmpEventList = 0;

	if (m_parallel)
	{
#ifndef DISABLEOPENMP
		std::cerr << "# Parallel time dependent mRNM: using " << omp_get_max_threads() << " threads" << std::endl;
		m_dtMinValues.resize(omp_get_max_threads());
		m_minPosValues.resize(m_dtMinValues.size());
#endif // !DISABLEOPENMP
	}
}

SimpleState::~SimpleState()
{
}

bool SimpleState::initEventTimes() const
{
	const std::vector<EventBase *> &events = getCurrentEvents();
	GslRandomNumberGenerator *pRndGen = getRandomNumberGenerator();

	// NOTE: this cannot be parallellized this way, unless multiple
	// random number generators are used. Shouldn't be the bottleneck 
	// anyway
	for (int i = 0 ; i < events.size() ; i++)
		events[i]->generateNewInternalTimeDifference(pRndGen, this);

	return true;
}

EventBase *SimpleState::getNextScheduledEvent(double &dt)
{
	// Get the currently enabled events
	// Note that when new events were added after firing one,
	// the generateNewInternalTimeDifference function should
	// already be called for them to generate the next step in
	// the unit-rate poisson process
	const std::vector<EventBase *> &events = getCurrentEvents();

	if (events.size() < 1)
	{
		setErrorString("The number of possible events became zero at a certain point");
		return 0;
	}

	// Find the minimal time that will go by
	double dtMin = std::numeric_limits<double>::max();
	double t = getTime();
	int eventPos = -1;
	int numEvents = events.size();

	if (m_parallel)
	{
#ifndef DISABLEOPENMP
		for (int i = 0 ; i < m_dtMinValues.size() ; i++)
		{
			m_dtMinValues[i] = dtMin;
			m_minPosValues[i] = eventPos;
		}

		#pragma omp parallel for
		for (int i = 0 ; i < events.size() ; i++)
		{
			double dt = events[i]->solveForRealTimeInterval(this, t);

			assert(dt >= 0);

			if (dt < m_dtMinValues[omp_get_thread_num()])
			{
				m_dtMinValues[omp_get_thread_num()] = dt;
				m_minPosValues[omp_get_thread_num()] = i;
			}
		}

		for (int i = 0 ; i < m_dtMinValues.size() ; i++)
		{
			if (m_dtMinValues[i] < dtMin)
			{
				dtMin = m_dtMinValues[i];
				eventPos = m_minPosValues[i];
			}
		}
#endif // DISABLEOPENMP
	}
	else
	{
		for (int i = 0 ; i < numEvents ; i++)
		{
			double dt = events[i]->solveForRealTimeInterval(this, t);

			assert(dt >= 0);

			if (dt < dtMin)
			{
				dtMin = dt;
				eventPos = i;
			}
		}
	}

	assert(eventPos >= 0 && eventPos < numEvents);
	assert(dtMin >= 0);

	m_eventPos = eventPos;
	m_pTmpEventList = &events;

	EventBase *pNextEvent = events[eventPos];
	dt = dtMin;

#ifdef STATE_SHOW_EVENTS
	showEvents();
#endif // STATE_SHOW_EVENTS

	return pNextEvent;
}

void SimpleState::advanceEventTimes(EventBase *pScheduledEvent, double dtMin)
{
	const std::vector<EventBase *> &events = *m_pTmpEventList;

	assert(pScheduledEvent == events[m_eventPos]);

	double t = getTime();
	double t1 = t + dtMin;
	int numEvents = events.size();

	// For this simple version we'll assume that _all_ next event times will need
	// to be recalculated every time

	if (m_parallel)
	{
#ifndef DISABLEOPENMP
		#pragma omp parallel for
		for (int i = 0 ; i < numEvents ; i++)
		{
			if (i != m_eventPos)
				events[i]->subtractInternalTimeInterval(this, t1);
		}
#endif // !DISABLEOPENMP
	}
	else
	{
		// TODO: handle the case in which multiple events fire at the same time?
		// ok, got the event position, we'll leave this out of this iteration
		// these need to be updated using the old propensities, before firing the event
		for (int i = 0 ; i < m_eventPos ; i++)
			events[i]->subtractInternalTimeInterval(this, t1);

		for (int i = m_eventPos+1 ; i < numEvents ; i++)
			events[i]->subtractInternalTimeInterval(this, t1);
	}
}

void SimpleState::onFiredEvent(EventBase *pEvt)
{
	onFiredEvent(pEvt, m_eventPos);
}

