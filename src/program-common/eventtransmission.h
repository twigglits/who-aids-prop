#ifndef EVENTTRANSMISSION_H

#define EVENTTRANSMISSION_H

#include "simpactevent.h"

class ConfigSettings;

class EventTransmission : public SimpactEvent
{
public:
	// Transmission from person1 onto person2
	EventTransmission(Person *pPerson1, Person *pPerson2);
	~EventTransmission();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static double getParamB()																		{ return s_b; }
	static double getParamC()																		{ return s_c; }

	static void infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless(const PopulationStateInterface &population) override;
	double calculateHazardFactor(const SimpactPopulation &population, double t0);

	static double s_a;
	static double s_b;
	static double s_c;
	static double s_d1;
	static double s_d2;
	static double s_f1;
	static double s_f2;
	static double s_tMaxAgeRefDiff;
};

#endif // EVENTTRANSMISSION_H

