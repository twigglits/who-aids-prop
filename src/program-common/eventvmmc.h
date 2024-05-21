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
    // getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	bool isEveryoneAffected() const									{ return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	// static bool m_VMMC_enabled;
	static ProbabilityDistribution *m_vmmcprobDist;
    static ProbabilityDistribution *m_pVMMC;
	
private:
	bool isEligibleForTreatment(double t, const State *pState);
    // double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen);
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	static ProbabilityDistribution *m_VMMCscheduleDist;
	

	static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
};

#endif // EVENTVMMC_H