#ifndef POPULATION_H

#define POPULATION_H

/**
 * \file population.h
 */

//#define SIMPLEMNRM -> now defined using the CMakeList.txt file

#include "simplestate.h"
#include "mutex.h"
#include <assert.h>

class GslRandomNumberGenerator;
class PersonBase;
class PopulationEvent;

/**
 * This class provides functions for a population-based simulation using
 * the modified Next Reaction Method (mNRM). Being population-based, the
 * state of the simulation mostly consists of the (living) people. 
 *
 * Depending on a compiler
 * setting, it either uses the very straightforward algorithm provided
 * by the SimpleState class, or it uses a different algorithm by deriving
 * from State directly and overriding the functions State::getNextScheduledEvent
 * and State::advanceEventTimes. It is always good to compare the results
 * of the two versions to assure the correct working of this more optimized 
 * version.
 *
 * This is a base class for an actual simulation and needs to be completed.
 * The initialization of the persons in the population (represented by
 * classes derived from PersonBase) and of the initial events in the simulation
 * needs to be done in a derived class. When this initialization is done,
 * the Population::run function can be called to actually start the simulation.
 *
 * Events for this type of simulation should derive from the PopulationEvent
 * class instead of using the EventBase base class directly.
 *
 * #### Advanced algorithm ####
 *
 * To know how to use this population based algorithm, it can be useful to understand
 * the way it works. The figure below illustrates how everything is organized. In
 * essence, a population is just a collection of people (represented by some class
 * derived from PersonBase), and each person stores a list of events that are relevant
 * to him. 
 *
 * ![](optalg.png)
 *
 * When you construct a new PopulationEvent based instance, you need to specify the
 * persons involved in this event, and the event gets stored in these persons' lists. As the figure
 * shows, it is very well possible that a single event appears in the lists of
 * different people: for example a relationship formation event would involve two
 * persons and would therefore be present in two lists. To be able to have global events,
 * events that in principle don't affect people that are known in advance, a 'dummy'
 * person is introduced. This 'dummy' person, neither labelled as a 'Man' nor as a 'Woman',
 * only has such global events in its event list. By definition, these events will not
 * be present in any other person's list. Note that this implies that PopulationEvent::getNumberOfPersons
 * will also return 1 for global events.
 *
 * When an event fires, the algorithm assumes that the persons which have the event
 * in their lists are affected and that their events will require a recalculation of
 * the fire times. In case other people are affected as well (who you don't know
 * beforehand), this can be specified using the functions PopulationEvent::isEveryoneAffected
 * or PopulationEvent::markOtherAffectedPeople. If such additional people are specified 
 * as well, those people's event fire times will be recalculated as well. Using PopulationEvent::areGlobalEventsAffected
 * you can indicate that the fire times of global events should be recalculated.
 *
 * Before recalculating an event fire time, it is checked if the event is still relevant.
 * If one of the persons specified in the PopulationEvent constructor has died, the
 * event is deemed useless and will be discarded. In case it's possible that an event
 * becomes useless because of some other criteria, the PopulationEvent::isUseless function
 * should be reimplemented to inform the algorithm about this. But note that this is only
 * called before recalculating an event fire time, which in turn is only done for people
 * affected by the event.
 *
 * Each person keeps track of which event in his list will fire first. To know which
 * event in the entire simulation will fire first, the algorithm then just needs to
 * check the first event times for all the people.
 */
#ifndef SIMPLEMNRM
class Population : public State
#else
class Population : public SimpleState
#endif // SIMPLEMNRM
{
public:
	/** Constructor of the class, indicating if a parallel version
	 *  should be used and which random number generator should be
	 *  used. */
	Population(bool parallel, GslRandomNumberGenerator *pRng);
	~Population();

	bool isParallel() const							{ return m_parallel; }
	
	/** This should be called to actually start the simulation, do not call
	 *  State::evolve for this.
	 *  \param tMax Stop the simulation if the simulation time exceeds the specified time. Upon
	 *              completion of the function, this variable will contain the actual simulation
	 *              time stopped.
	 *  \param maxEvents If positive, the simulation will stop if this many events have been
	 *                   executed. Set to a negative value to disable this limit. At the end of
	 *                   the simulation, this variable will contain the number of events executed.
	 *  \param startTime The start time of the simulation, can be used to continue where a previous
	 *                   call to this function left off.
	 */
	bool run(double &tMax, int64_t &maxEvents, double startTime = 0)	{ return State::evolve(tMax, maxEvents, startTime, false); }

	/** Returns a list to the current living members in the population, introduced
	 *  into the simulation using Population::addNewPerson. */
	PersonBase **getAllPeople()						{ if (m_people.size() == m_numGlobalDummies) return 0; return &(m_people[m_numGlobalDummies]); }

	/** Same as Population::getAllPeople, but only the men are returned. */
	PersonBase **getMen();

	/** Same as Population::getAllPeople, but only the women are returned. */
	PersonBase **getWomen();

	/** Returns the people who were part of the simulation but who are now deceased
	 *  (intended for result analysis, not to be used during the simulation). */
	PersonBase **getDeceasedPeople()					{ if (m_deceasedPersons.size() == 0) return 0; return &(m_deceasedPersons[0]); }

	/** Returns the number of people in the array returned by Population::getAllPeople. */
	int getNumberOfPeople() const						{ int num = (int)m_people.size() - m_numGlobalDummies; assert(num >= 0); return num; }

	/** Returns the number of people in the array returned by Population::getMen. */
	int getNumberOfMen() const						{ return m_numMen; }

	/** Returns the number of people in the array returned by Population::getWomen. */
	int getNumberOfWomen() const						{ return m_numWomen; }

	/** Returns the number of people in the array returned by Population::getDeceasedPeople. */
	int getNumberOfDeceasedPeople() const					{ return m_deceasedPersons.size(); }

	/** When a new person is introduced into the population, this function must be
	 *  used to tell the simulation about this. In essence this function will make
	 *  sure that the person appears in the arrays returned by Population::getAllPeople,
	 *  Population::getMen and Population::getWomen. */
	void addNewPerson(PersonBase *pPerson);

	/** When a person has died, this function must be called to inform the simulation about
	 *  this. This function will set the time of death for the person and will remove the
	 *  person from the arrays with living people (Population::getAllPeople, Population::getMen
	 *  and Population::getWomen. */
	void setPersonDied(PersonBase *pPerson);

	/** When a new event has been created, it must be injected into the simulation using
	 *  this function. */
	void onNewEvent(PopulationEvent *pEvt);

	/** This should only be called from within the PopulationEvent::markOtherAffectedPeople function,
	 *  to indicate which other persons are also affected by the event (other that the persons
	 *  mentioned in the event constructor. */
	void markAffectedPerson(PersonBase *pPerson) const;

	// TODO: shield these from the user somehow? These functions should not be used
	//       directly by the user, they are used internally by the algorithm
	void scheduleForRemoval(PopulationEvent *pEvt);
	void lockEvent(PopulationEvent *pEvt) const;
	void unlockEvent(PopulationEvent *pEvt) const;
	void lockPerson(PersonBase *pPerson) const;
	void unlockPerson(PersonBase *pPerson) const;
private:
	bool initEventTimes() const;
#ifndef SIMPLEMNRM
	EventBase *getNextScheduledEvent(double &dt);
	void advanceEventTimes(EventBase *pScheduledEvent, double dt);
#else
	const std::vector<EventBase *> &getCurrentEvents() const					{ return m_allEvents; }
	void onFiredEvent(EventBase *pEvt, int position);

	std::vector<EventBase *> m_allEvents;
#endif // SIMPLEMNRM

#ifdef STATE_SHOW_EVENTS
	void showEvents(); // FOR DEBUGGING
#endif // STATE_SHOW_EVENTS
	void onAlgorithmLoop();

	PopulationEvent *getEarliestEvent(const std::vector<PersonBase *> &people);
	int64_t getNextEventID();
	int64_t getNextPersonID();
	
	// These are living persons, the first part men, the second are women
	std::vector<PersonBase *> m_people;
	int m_numMen, m_numWomen;
	int m_numGlobalDummies;

	// Deceased persons
	std::vector<PersonBase *> m_deceasedPersons;

#ifndef DISABLEOPENMP
	Mutex m_eventsToRemoveMutex;
#endif // !DISABLEOPENMP
	std::vector<EventBase *> m_eventsToRemove;

	// For the parallel version
	bool m_parallel;

	int64_t m_nextEventID, m_nextPersonID;
#ifndef DISABLEOPENMP
	Mutex m_nextEventIDMutex, m_nextPersonIDMutex;
#endif // !DISABLEOPENMP

	std::vector<PopulationEvent *> m_tmpEarliestEvents;
	std::vector<double> m_tmpEarliestTimes;

	mutable std::vector<PersonBase *> m_otherAffectedPeople;

#ifndef DISABLEOPENMP
	mutable std::vector<Mutex> m_eventMutexes;
	mutable std::vector<Mutex> m_personMutexes;
#endif // !DISABLEOPENMP
};

inline int64_t Population::getNextEventID()
{
#ifndef DISABLEOPENMP
	if (m_parallel)
		m_nextEventIDMutex.lock();
#endif // !DISABLEOPENMP
	
	int64_t id = m_nextEventID++;

#ifndef DISABLEOPENMP
	if (m_parallel)
		m_nextEventIDMutex.unlock();
#endif // !DISABLEOPENMP

	return id;
}

inline int64_t Population::getNextPersonID()
{
#ifndef DISABLEOPENMP
	if (m_parallel)
		m_nextPersonIDMutex.lock();
#endif // !DISABLEOPENMP
	
	int64_t id = m_nextPersonID++;

#ifndef DISABLEOPENMP
	if (m_parallel)
		m_nextPersonIDMutex.unlock();
#endif // !DISABLEOPENMP

	return id;
}

#ifdef NDEBUG

inline void Population::markAffectedPerson(PersonBase *pPerson) const
{ 
	m_otherAffectedPeople.push_back(pPerson); 
}

inline PersonBase **Population::getMen()
{
	if (m_numMen == 0)
		return 0;

	return &(m_people[m_numGlobalDummies]);
}

inline PersonBase **Population::getWomen()
{
	if (m_numWomen == 0)
		return 0;

	return &(m_people[m_numMen+m_numGlobalDummies]);
}

#endif // NDEBUG

#endif // POPULATION_H
