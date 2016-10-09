#ifndef EVENTMTCTRANSMISSION_H

#define EVENTMTCTRANSMISSION_H

#include "simpactevent.h"

class EventMTCTransmission : public SimpactEvent
{
public:
	EventMTCTransmission(Person *pMother, Person *pChild);
	~EventMTCTransmission();

	std::string getDescription(double tNow) const;
	void fire(State *pState, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless();
};

#endif // EVENTMTCTRANSMISSION_H

