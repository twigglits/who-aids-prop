#ifndef EVENTTEST_H

#define EVENTTEST_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventTest : public SimpactEvent
{
public:
	EventTest(Person *pPerson);
	~EventTest();

	std::string getDescription(double tNow);
	void writeLogs(double tNow) const;
	void fire(State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	bool isEligibleForTreatment(double t);
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);

	static double s_treatmentVLLogFrac;
	static double s_cd4Threshold;
	static ProbabilityDistribution *s_pTestingDistribution;
};

#endif // EVENTTEST_H

