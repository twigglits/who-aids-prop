#ifndef EVENTDISSOLUTION_H

#define EVENTDISSOLUTION_H

#include "simpactevent.h"

class ConfigSettings;

class EventDissolution : public SimpactEvent
{
public:
	EventDissolution(Person *pPerson1, Person *pPerson2, double formationTime);
	~EventDissolution();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);

	double m_formationTime;

	static double getTMax(Person *pPerson1, Person *pPerson2);

	static double a0;		// baseline_factor
	static double a1;		// male_current_relations_factor   -> just current_relations_factor ?
	static double a2;		// female_current_relations_factor -> just current_relations_factor ?
	static double a3;		// current_relations_difference_factor?? TODO can't find this
	static double a4;		// mean_age_factor
	static double a5;		// age_difference_factor
	static double Dp;		// preferred_age_difference
	static double b;		// last_change_factor
	static double tMaxDiff;
};

#endif // EVENTDISSOLUTION_H

