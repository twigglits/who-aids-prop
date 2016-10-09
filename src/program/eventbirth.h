#ifndef EVENTBIRTH_H

#define EVENTBIRTH_H

#include "simpactevent.h"

class Person;
class Man;

class EventBirth : public SimpactEvent
{
public:
	EventBirth(Person *pPerson);
	~EventBirth();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);

	void setFather(Person *pFather);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	Man *m_pFather;
};

#endif // EVENTBIRTH_H
