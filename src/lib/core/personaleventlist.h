#ifndef PERSONALEVENTLIST_H

#define PERSONALEVENTLIST_H

#include "errut/errorbase.h"
#include "populationevent.h"
#include "state.h"
#include <vector>
#include <list>
#include <set>

//#define PERSONALEVENTLIST_EXTRA_DEBUGGING

class PersonBase;

class PersonalEventList : public errut::ErrorBase
{
public:
	PersonalEventList(PersonBase *pPerson);
	~PersonalEventList();

	void registerPersonalEvent(PopulationEvent *pEvt);
	void processUnsortedEvents(Population &pop, double t0);
	void advanceEventTimes(const Population &pop, double t1);
	void adjustingEvent(PopulationEvent *pEvt);
	void removeTimedEvent(PopulationEvent *pEvt);

	PopulationEvent *getEarliestEvent();
private:
#ifndef PERSONALEVENTLIST_EXTRA_DEBUGGING
	void checkEarliestEvent() { }
	void checkEvents() { }
#else   
	void checkEarliestEvent(); 
	void checkEvents();
#endif // PERSONALEVENTLIST_EXTRA_DEBUGGING

	std::vector<PopulationEvent *> m_timedEvents;
	std::vector<PopulationEvent *> m_untimedEvents;
	
	PopulationEvent *m_pEarliestEvent;
	PersonBase *m_pPerson;

#ifdef STATE_SHOW_EVENTS
	friend class Population;
#endif // STATE_SHOW_EVENTS
};

#endif // PERSONALEVENTLIST_H

