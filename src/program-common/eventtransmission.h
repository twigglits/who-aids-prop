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

	// TODO: access functions
	static double m_a;
	static double m_b;
	static double m_c;
	static double m_d1;
	static double m_d2;

	static void infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless();
	double calculateHazardFactor();
};

#endif // EVENTTRANSMISSION_H

