#ifndef EVENTDVRDROP_H
#define EVENTDVRDROP_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class PieceWiseLinearFunction;

class EventDVRDROP : public SimpactEvent
{
public:
	EventDVRDROP(Person *pPerson);
	~EventDVRDROP();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);
	bool isEveryoneAffected() const { return false; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	static ProbabilityDistribution *m_DVRDROPprobDist;
	static double s_DVRDROPThreshold;
	static int s_DVRDROPScheduleMax;
	static int s_DVRDROPScheduleMin;
    static bool m_DVRDROP_enabled;

private:
	bool isEligibleForTreatment(double t, const State *pState, Person *pPerson);
	bool isHardDropOut(double t, const State *pState, Person *pPerson);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen, Person *pPerson, const State *pState);
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
    static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
    static PieceWiseLinearFunction *s_pRecheckInterval;
};

#endif // EVENTDVRDROP_H