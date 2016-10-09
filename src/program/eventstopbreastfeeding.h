#ifndef EVENTSTOPBREASTFEEDING_H

#define EVENTSTOPBREASTFEEDING_H

#include "simpactevent.h"

class EventStopBreastFeeding : public SimpactEvent
{
public:
	EventStopBreastFeeding(Person *pPerson);
	~EventStopBreastFeeding();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
};

#endif // EVENTSTOPBREASTFEEDING_H

