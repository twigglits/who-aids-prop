#ifndef EVENTTEST_H

#define EVENTTEST_H

#include "simpactevent.h"

class EventTest : public SimpactEvent
{
public:
	EventTest(Person *pMother);
	~EventTest();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
    static bool hasNextIntervention();
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	static void popNextInterventionInfo(double &t, ConfigSettings &config);

	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
    static double getNextInterventionTime();
    static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
};

#endif // EVENTTEST_H
