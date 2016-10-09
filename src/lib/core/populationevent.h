#ifndef POPULATIONEVENT_H

#define POPULATIONEVENT_H

/**
 * \file populationevent.h
 */

#include "eventbase.h"
#include "population.h"
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#define POPULATIONEVENT_MAXPERSONS								2

class PersonBase;

/** This is the base class for events in population-based simulations which
 *  use the Population class. Such an event should always specify at creation
 *  time which people are always involved in the event. For example, one
 *  person is always involved in a 'mortality' event, and two persons will
 *  be involved in an event for the formation of a relationship. 
 *
 *  Of course, it is also possible that some people are affected because of
 *  an event, but that these people were not yet known at the time the event
 *  was created. For example, the hazard of a relationship formation event
 *  could depend on the number of relationships a person is in. When someone
 *  dies (i.e. a morality event), this ends all relationships that person was
 *  involved in, so all these people are affected by the event as well (they
 *  will need to recalculate event times since their number of relationships
 *  will have changed).
 *
 *  For this purpose, the following functions can be implemented, if not it
 *  means that no extra people are affected:
 *
 *  - PopulationEvent::getNumberOfOtherAffectedPersons: this should return the number of persons
 *    that are also affected by the event (and were not specified in the
 *    constructor). If this is negative, it means that all people in the entire
 *    population are affected (due the adjustment of some global property that's
 *    used in the hazard calculations). Of course, avoiding this is a very
 *    good idea, since having to recalculate everything will slow things down.
 *  - PopulationEvent::startOtherAffectedPersonIteration: to iterate over the persons that are
 *    affected, this function will be called a the start of this iteration.
 *  - PopulationEvent::getNextOtherAffectedPerson: when the iteration is started, calling this
 *    function should always return the next affected person. This will be
 *    called \c getNumberOfOtherAffectedPersons times, and if it is called more than
 *    that (so no more persons are available), the function should return 0.
 *
 *  The people specified in the constructor of the class should not be returned
 *  or counted by these functions, they are automatically taken into account. When
 *  one of the people specified in the constructor has died, the event will be
 *  considered useless in the rest of the simulation and will be discarded. Other
 *  conditions which can cause an event to become useless can be checked in the
 *  optional function PopulationEvent::isUseless.
 */
class PopulationEvent : public EventBase
{
public:
	PopulationEvent();

	/** Constructs an event relevant to one person. */
	PopulationEvent(PersonBase *pPerson);

	/** Constructs an event relevant to two persons. */
	PopulationEvent(PersonBase *pPerson1, PersonBase *pPerson2);
	~PopulationEvent();

	void setEventID(int64_t id)								{ assert(m_eventID < 0); assert(id >= 0); m_eventID = id; }
	int64_t getEventID() const								{ return m_eventID; }

	/** Returns the number of people specified during the creation of
	 *  the event. */
	int getNumberOfPersons() const								{ assert(m_numPersons >= 0 && m_numPersons <= POPULATIONEVENT_MAXPERSONS); return (int)m_numPersons; }

	/** Returns a person that was specified when the event was constructed,
	 *  where \c idx can range from 0 to PopulationEvent::getNumberOfPersons() - 1. */
	PersonBase *getPerson(int idx) const;

	/** Returns the number of persons that are also affected by the firing of the event
	 *  (see the detailed description of the class for more info). */
	virtual int getNumberOfOtherAffectedPersons() const					{ return 0; }

	/** Starts the iteration of the other affected persons. */
	virtual void startOtherAffectedPersonIteration()					{ }

	/** Retrieves an affected person and advaces the iteration (should return 0 if
	 *  no more persons are affected). */
	virtual PersonBase *getNextOtherAffectedPerson()					{ return 0; }

	/** Returns a short description of the event, can be useful for logging/debugging
	 *  purposes. */
	virtual std::string getDescription(double tNow) const					{ return std::string("No description given"); }

	// TODO: shield these from the user somehow? These functions are used internally
	//       by the algorithm and should not be called directly by the user. */
	void setEventIndex(PersonBase *pPerson, int idx);
	int getEventIndex(PersonBase *pPerson) const;

	void setScheduledForRemoval()								{ m_scheduledForRemoval = true; }
	bool isScheduledForRemoval() const							{ return m_scheduledForRemoval; }

	bool isNoLongerUseful();
protected:
	/** This function can be used to inform the algorithm that an event is no longer
	 *  of any use and should be discarded. An event is automatically useless if one of the
	 *  people specified at time of construction has died, so this function should not check
	 *  for that again. */
	virtual bool isUseless()								{ return false; } 
private:
	void commonConstructor();

	PersonBase *m_pPersons[POPULATIONEVENT_MAXPERSONS];
	int m_eventIndex[POPULATIONEVENT_MAXPERSONS];
	int8_t m_numPersons; 

	bool m_scheduledForRemoval;
	int64_t m_eventID;
};

inline void PopulationEvent::setEventIndex(PersonBase *pPerson, int idx)
{
	assert(pPerson != 0);
	int num = m_numPersons;

	for (int i = 0 ; i < num ; i++)
	{
		assert(m_pPersons[i] != 0);

		if (pPerson == m_pPersons[i])
		{
			m_eventIndex[i] = idx;
			return;
		}
	}
	
	std::cerr << "PopulationEvent::setEventIndex: Consistency error: invalid Person object in setEventIndex" << std::endl;
	exit(-1);
}

inline int PopulationEvent::getEventIndex(PersonBase *pPerson) const
{
	assert(pPerson != 0);
	int num = m_numPersons;

	for (int i = 0 ; i < num ; i++)
	{
		assert(m_pPersons[i] != 0);

		if (pPerson == m_pPersons[i])
			return m_eventIndex[i];
	}

	std::cerr << "PopulationEvent::getEventIndex: Consistency error: invalid Person object in getEventIndex" << std::endl;
	exit(-1);
}

#ifdef NDEBUG
inline PersonBase *PopulationEvent::getPerson(int idx) const
{ 
	return m_pPersons[idx];
}
#endif

#endif // POPULATIONEVENT_H
