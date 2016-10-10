#ifndef EVENTSTUDYSTEP_H

#define EVENTSTUDYSTEP_H

#include "simpactevent.h"

class ConfigSettings;

class EventStudyStep : public SimpactEvent
{
public:
	EventStudyStep(int stepIndex);
	~EventStudyStep();

	std::string getDescription(double tNow) const;
	void writeLogs(const Population &pop, double tNow) const;

	void fire(State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);

	static double getStepInterval()															{ return s_stepInterval; }
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	int m_stepIndex;

	static double s_stepInterval;
};

#endif // EVENTSTUDYSTEP_H

