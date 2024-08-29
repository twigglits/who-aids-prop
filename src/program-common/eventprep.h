#ifndef EVENTPREP_H
#define EVENTPREP_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class PieceWiseLinearFunction;

class EventPrep : public SimpactEvent
{
public:
	EventPrep(Person *pPerson1, bool scheduleImmediately = false);
	~EventPrep();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
	bool isEveryoneAffected() const { return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
    static bool m_prep_enabled;
	static ProbabilityDistribution *m_prepprobDist;
	static double s_prepThreshold;
	static double s_AGYWThreshold;

private:
	bool isEligibleForTreatmentP1(double t, const State *pState);
	bool isWillingToStartTreatmentP1(double t, GslRandomNumberGenerator *pRndGen);
    bool m_scheduleImmediately;
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
    static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
    static PieceWiseLinearFunction *s_pRecheckInterval;
};

#endif // EVENTPREP_H
