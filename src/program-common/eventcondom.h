#ifndef EVENTCONDOM_H
#define EVENTCONDOM_H

#include "simpactevent.h"
#include "configsettings.h"
#include <list>

class EventCondom : public SimpactEvent
{
public:
	EventCondom(Person *pPerson);
	~EventCondom();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
    bool isEnabled() const { return m_enabled; } // Add this method to check if the event is enabled
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool hasNextIntervention();
	static ProbabilityDistribution *m_condomprobDist;
	static ProbabilityDistribution *m_condomscheduleDist;
	static bool m_condom_enabled;
    static double s_condomThreshold; // New static variable for the threshold


private:
	bool isEligibleForTreatment(double t, const State *pState);
	bool isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen);

	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState, bool initializationPhase);

	static std::list<double> m_interventionTimes;
	static std::list<ConfigSettings> m_interventionSettings;
	static bool m_interventionsProcessed;
    static bool m_enabled; // Add this static member
};

#endif // EVENTCONDOM_H