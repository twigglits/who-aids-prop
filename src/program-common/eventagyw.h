#ifndef EVENTAGYW_H
#define EVENTAGYW_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class EventAGYW : public SimpactEvent
{
public:
	EventAGYW(Person *pPerson);
	~EventAGYW();

	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
	bool isEveryoneAffected() const { return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	static ProbabilityDistribution *m_agywprobDist;
    static ProbabilityDistribution *m_pAGYW;
	static bool m_AGYW_enabled; 
	static double s_agywThreshold;

private:
	bool isEligibleForTreatment(double t, const State *pState);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen);
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	static ProbabilityDistribution *m_agywscheduleDist;

	static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
};

#endif // EVENTAGYW_H
