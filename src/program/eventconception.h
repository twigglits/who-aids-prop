#ifndef EVENTCONCEPTION_H

#define EVENTCONCEPTION_H

#include "simpactevent.h"

class EventConception : public SimpactEvent
{
public:
	EventConception(Person *pPerson1, Person *pPerson2);
	~EventConception();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless();
};

#endif // EVENTCONCEPTION_H

