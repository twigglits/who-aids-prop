#ifndef EVENTSTUDYEND_H

#define EVENTSTUDYEND_H

#include "simpactevent.h"

class ConfigSettings;

class EventStudyEnd : public SimpactEvent
{
public:
	EventStudyEnd();
	~EventStudyEnd();

	std::string getDescription(double tNow) const;
	void writeLogs(const Population &pop, double tNow) const;

	void fire(State *pState, double t);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
};

#endif // EVENTSTUDYEND_H

