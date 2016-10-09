#ifndef STATE_H

#define STATE_H

/** 
 * \file state.h
 */

#include "errut/errorbase.h"
#include <stdint.h>
#include <vector>

//#define STATE_SHOW_EVENTS

class GslRandomNumberGenerator;
class EventBase;

/**
 *  This class both describes the simulation state and contains the core algorithm
 *  (as shown on the main page of the documentation) to execute the modified next
 *  reaction method (mNRM). Using this
 *  class alone will not work however, since it does not contain an implementation of
 *  several necessary functions. At leest three functions must be implemented in a 
 *  subclass:
 *
 *   - initEventTimes: if requested, this is called at the start of the algorithm to
 *                     calculate initial internal event fire times.
 *   - getNextScheduledEvent: this function should not only calculate the real world
 *                            event fire times for events which still need the mapping
 *                            from internal to real world time, but it should also
 *                            calculate the real world time that will elapse until the next
 *                            event.
 *   - advanceEventTimes: this function should diminish the internal clocks of the relevant
 *                        events, corresponding to the real world time interval until
 *                        the event fire time.
 *
 *  A very straightforward implementation is available in the SimpleState class, where
 *  no attempt is made to avoid unnecessary recalculations.
 */
class State : public errut::ErrorBase
{
public:
	/** Constructor of the class, to which the random number generator to be used internally
	 *  must be specified. */
	State(GslRandomNumberGenerator *pRng);
	~State();

	/** This advances the simulation state using the core mNRM.
	 *
	 *  \param tMax Stop the simulation if the simulation time exceeds the specified time. Upon
	 *              completion of the function, this variable will contain the actual simulation
	 *              time stopped.
	 *  \param maxEvents If positive, the simulation will stop if this many events have been
	 *                   executed. Set to a negative value to disable this limit. At the end of
	 *                   the simulation, this variable will contain the number of events executed.
	 *  \param startTime The start time of the simulation, can be used to continue where a previous
	 *                   call to this function left off.
	 *  \param initEvents If set to true, the State::initEvents function will be called before entering
	 *                    the algorithm loop.
	 *
	 *  The algorithm executed is the following:
	 *  @code
	 *  if (initEvents)
	 *  	initEventTimes();
	 *  
	 *  bool done = false;
	 *  int64_t eventCount = 0;
	 *  m_time = startTime;
	 *  
	 *  while (!done)
	 *  {
	 *  	double dtMin = -1;
	 *  	EventBase *pNextScheduledEvent = getNextScheduledEvent(dtMin);
	 *  
	 *  	if (pNextScheduledEvent == 0)
	 *  		return false;
	 *  
	 *  	advanceEventTimes(pNextScheduledEvent, dtMin);
	 *  
	 *  	m_time += dtMin;
	 *  	onAboutToFire(pNextScheduledEvent);
	 *  	pNextScheduledEvent->fire(this, m_time);
	 *  
	 *  	// If the event is still being used (the default) we'll need a new random number
	 *  	if (!pNextScheduledEvent->willBeRemoved())
	 *  		pNextScheduledEvent->generateNewInternalTimeDifference(m_pRndGen, this);
	 *  
	 *  	eventCount++;
	 *  
	 *  	if (m_time > tMax || (maxEvents > 0 && eventCount >= maxEvents))
	 *  		done = true;
	 *  
	 *  	onFiredEvent(pNextScheduledEvent);
	 *  	onAlgorithmLoop(done);
	 *  }
	 *  
	 *  tMax = m_time;
	 *  maxEvents = eventCount;
	 *  
	 *  return true;
	 *  @endcode
	 *
	 *  Apart from the core functions State::initEventTimes, State::getNextScheduledEvent and
	 *  State::advanceEventTimes which need to be provided by an implementation to get a working
	 *  algorithm, a few extra functions can come in handy as well:
	 *
	 *   - onAboutToFire: called right before an event will fire
	 *   - onFiredEvent: called right after an event fired
	 *   - onAboutToFire: called when the algoritm is going to loop
	 */
	bool evolve(double &tMax, int64_t &maxEvents, double startTime = 0, bool initEvents = true);

	/** This function returns the current time of the simulation. */
	double getTime() const										{ return m_time; }

	/** Returns the random number generator that was specified in the constructor. */
	GslRandomNumberGenerator *getRandomNumberGenerator() const					{ return m_pRndGen; }
protected:
	/** Generate the internal times for the events present in the algorithm (called
	 *  by State::evolve depending on the value of the initEvents parameter). */
	virtual bool initEventTimes() const;

	/** Return the next event to be fired and store the real world time that will
	 *  have passed until it fires in \c dt. */
	virtual EventBase *getNextScheduledEvent(double &dt);

	/** Advance the times of the necessary events to the time when \c dt has passed,
	 *  ignoring pScheduledEvent since this is the one we will be firing. */
	virtual void advanceEventTimes(EventBase *pScheduledEvent, double dt);

	/** Called right before pEvt is fired. */
	virtual void onAboutToFire(EventBase *pEvt)							{ }

	/** Called after pEvt is fired. */
	virtual void onFiredEvent(EventBase *pEvt)							{ }

	/** Called at the end of each algorithm loop, with \c finished set to true if
	 *  the loop will be exited. */
	virtual void onAlgorithmLoop(bool finished)							{ }
private:
	mutable GslRandomNumberGenerator *m_pRndGen;
	double m_time;
};

#endif // STATE_H

