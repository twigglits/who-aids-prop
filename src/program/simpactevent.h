#ifndef SIMPACTEVENT_H

#define SIMPACTEVENT_H

#include "populationevent.h"
#include "simpactpopulation.h" 
#include "person.h"

// This just provides some casts towards Person instead of PersonBase
class SimpactEvent : public PopulationEvent
{
public:
	SimpactEvent() : PopulationEvent()							{ }
	SimpactEvent(Person *pPerson) : PopulationEvent(pPerson)				{ }
	SimpactEvent(Person *pPerson1, Person *pPerson2) : PopulationEvent(pPerson1, pPerson2)	{ }
	~SimpactEvent()										{ }

	Person *getPerson(int idx) const							{ return static_cast<Person*>(PopulationEvent::getPerson(idx)); }
};

#endif // SIMPACTEVENT_H

