#ifndef EVENTDVR_H
#define EVENTDVR_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class PieceWiseLinearFunction;

class EventDVR : public SimpactEvent
{
public:
	EventDVR(Person *pPerson);
	~EventDVR();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
	bool isEveryoneAffected() const { return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	static ProbabilityDistribution *m_DVRprobDist;
	static double s_DVRThreshold;
    static bool m_DVR_enabled;

private:
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

#endif // EVENTDVR_H