#ifndef EVENTHIVSEED_H

#define EVENTHIVSEED_H

#include "simpactevent.h"

class ConfigSettings;

class EventHIVSeed : public SimpactEvent
{
public:
	EventHIVSeed();
	~EventHIVSeed();

	std::string getDescription(double tNow) const;
	void writeLogs(const Population &pop, double tNow) const;

	void fire(State *pState, double t);
	
	// This can be made more fine grained, but for now this should suffice
	bool isEveryoneAffected() const									{ return true; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static double getSeedTime()									{ return m_seedTime; }
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static double m_seedTime, m_seedFraction;
	static double m_seedMinAge, m_seedMaxAge;
	static int m_seedAmount;
	static bool m_stopOnShort;
	static bool m_useFraction;

	static bool m_seeded;
};

#endif // EVENTHIVSEED_H

