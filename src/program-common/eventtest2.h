#ifndef EVENTTEST2_H

#define EVENTTEST2_H

#include "simpactevent.h"

class EventTest2 : public SimpactEvent
{
public:
	EventTest2(Person *pMother);
	~EventTest2();

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

#endif // EVENTTEST2_H