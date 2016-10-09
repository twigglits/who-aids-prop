#ifndef EVENTMORTALITY_H

#define EVENTMORTALITY_H

#include "simpactevent.h"

// Non-AIDS (normal) mortality
class EventMortality : public SimpactEvent
{
public:
	EventMortality(Person *pPerson);
	~EventMortality();

	bool isAidsMortality() const						{ return m_isAidsMortality; }

	std::string getDescription(double tNow) const;

	int getNumberOfOtherAffectedPersons() const;
	void startOtherAffectedPersonIteration();
	PersonBase *getNextOtherAffectedPerson();

	void fire(State *pState, double t);
private:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	bool isUseless();

	bool m_isAidsMortality;
};

#endif // EVENTMORTALITY_H
