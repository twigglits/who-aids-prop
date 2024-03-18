#ifndef EVENTVMMC_H

#define EVENTVMMC_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class EventVMMC : public SimpactEvent
{
public:
	EventVMMC(Person *pPerson);
	~EventVMMC();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	bool isEveryoneAffected() const									{ return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static double getNextInterventionTime();
	static void popNextInterventionInfo(double &t, ConfigSettings &config);

	bool isEligibleForTreatment(double t);
	bool isWillingToStartTreatment();

	bool m_scheduleImmediately;

	static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
};

#endif // EVENTVMMC_H