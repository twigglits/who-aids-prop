#ifndef EVENTPREPDROP_H

#define EVENTPREPDROP_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventPrepDrop : public SimpactEvent
{
public:
	EventPrepDrop(Person *pPerson, double t);
	~EventPrepDrop();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
    bool dropOutFraction(double t, GslRandomNumberGenerator *pRndGen);
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	// double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);

	// double m_treatmentStartTime;

	static ProbabilityDistribution *s_prepDropDistribution;
};

#endif // EVENTPREPDROP_H

