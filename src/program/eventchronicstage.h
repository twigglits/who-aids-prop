#ifndef EVENTCHRONICSTAGE_H

#define EVENTCHRONICSTAGE_H

#include "simpactevent.h"

class EventChronicStage : public SimpactEvent
{
public:
	EventChronicStage(Person *pPerson);
	~EventChronicStage();

	std::string getDescription(double tNow) const;

	void fire(State *pState, double t);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
};

#endif // EVENTCHRONICSTAGE_H
