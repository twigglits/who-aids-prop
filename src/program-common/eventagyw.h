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

    std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

    static double getAGYWAge()								{ return m_AGYW_age; }
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static bool m_AGYW_enabled; 
private:
    static double m_AGYW_age;
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);	
};

#endif // EVENTAGYW_H
