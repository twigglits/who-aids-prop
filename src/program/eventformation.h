#ifndef EVENTFORMATION_H

#define EVENTFORMATION_H

#include "simpactevent.h"

class ConfigSettings;
class EvtHazard;

class EventFormation : public SimpactEvent
{
public:
	// set last dissolution time to -1 if irrelevant
	EventFormation(Person *pPerson1, Person *pPerson2, double lastDissTime);
	~EventFormation();

	std::string getDescription(double tNow) const;
	void writeLogs(double tNow) const;
	void fire(State *pState, double t);

	double getLastDissolutionTime() const								{ return m_lastDissolutionTime; }

	static void processConfig(ConfigSettings &config);
	static void obtainConfig(ConfigWriter &config);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless();

	double m_lastDissolutionTime;

	static EvtHazard *m_pHazard;
};

#endif // EVENTFORMATION_H

