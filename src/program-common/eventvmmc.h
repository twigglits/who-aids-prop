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
    bool isEnabled();
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	bool isEveryoneAffected() const									{ return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	// static bool m_VMMC_enabled;
	static ProbabilityDistribution *m_vmmcprobDist;
	
private:
	bool isEligibleForTreatment(double t);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen);
	

	static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
};

#endif // EVENTVMMC_H