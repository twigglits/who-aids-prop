#ifndef EVENTDEBUT_H

#define EVENTDEBUT_H

#include "simpactevent.h"

class ConfigSettings;

class EventDebut : public SimpactEvent
{
public:
	EventDebut(Person *pPerson);
	~EventDebut();

	std::string getDescription(double tNow) const;
	void writeLogs(double tNow) const;

	void fire(State *pState, double t);

	static double getDebutAge()								{ return m_debutAge; }
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static double m_debutAge;
};

#endif // EVENTDEBUT_H
