#ifndef EVENTHIVTEST_H

#define EVENTHIVTEST_H

#include "simpactevent.h"

class EventHIVTest : public SimpactEvent
{
public:
	EventHIVTest(Person *pPerson);
	~EventHIVTest();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	bool isUseless();
};


#endif // EVENTHIVTEST_H
