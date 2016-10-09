#ifndef EVENTTRANSMISSION_H

#define EVENTTRANSMISSION_H

#include "simpactevent.h"

class EventTransmission : public SimpactEvent
{
public:
	EventTransmission(Person *pPerson1, Person *pPerson2);
	~EventTransmission();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless();
};

#endif // EVENTTRANSMISSION_H

