#ifndef EVENTCABDROP_H
#define EVENTCABDROP_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class PieceWiseLinearFunction;

class EventCABDROP : public SimpactEvent
{
public:
	EventCABDROP(Person *pPerson);
	~EventCABDROP();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
	bool isEveryoneAffected() const { return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	static ProbabilityDistribution *m_CABDROPprobDist;
	static ProbabilityDistribution *m_CABDROPschedDist;
	static double s_CABDropThreshold;
    static bool m_CABDrop_enabled;

private:
	bool isHardDropOut(double t, const State *pState, Person *pPerson);
	bool isEligibleForTreatment(double t, const State *pState, Person *pPerson);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson);
    bool m_scheduleImmediately;

    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	
    static ProbabilityDistribution *m_prepscheduleDist;
    
    static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
    static PieceWiseLinearFunction *s_pRecheckInterval;
};

#endif // EVENTCABDROP_H