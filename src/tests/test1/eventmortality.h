#ifndef EVENTMORTALITY_H

#define EVENTMORTALITY_H

#include "simpactevent.h"

// Non-AIDS (normal) mortality
class EventMortality : public SimpactEvent
{
public:
	EventMortality(Person *pPerson);
	~EventMortality();

	std::string getDescription(double tNow) const;

	int getNumberOfOtherAffectedPersons() const;
	void startOtherAffectedPersonIteration();
	PersonBase *getNextOtherAffectedPerson();

	void fire(Algorithm *pAlgorithm, State *pState, double t);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
};

#endif // EVENTMORTALITY_H
