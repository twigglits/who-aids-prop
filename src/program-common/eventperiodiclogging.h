#ifndef EVENTPERIODICLOGGING_H

#define EVENTPERIODICLOGGING_H

#include "simpactevent.h"
#include "logfile.h"

class ConfigSettings;

// This is a global event, but nobody is affected (nothing changes),
// just some stats are written
class EventPeriodicLogging : public SimpactEvent
{
public:
	EventPeriodicLogging();
	~EventPeriodicLogging();

	std::string getDescription(double tNow) const;
	void writeLogs(double tNow) const;

	void fire(State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);

	static bool isEnabled() 								{ return (s_loggingInterval > 0); }
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static LogFile s_logFile;
	static std::string s_logFileName;
	static double s_loggingInterval;
};

#endif // EVENTPERIODICLOGGING_H

