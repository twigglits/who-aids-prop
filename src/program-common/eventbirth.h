#ifndef EVENTBIRTH_H

#define EVENTBIRTH_H

#include "simpactevent.h"

class EventBirth : public SimpactEvent
{
public:
	EventBirth(Person *pMother);
	~EventBirth();

	std::string getDescription(double tNow) const;
	void writeLogs(const Population &pop, double tNow) const;
	void fire(State *pState, double t);

	void setFather(Person *pFather);

	// If the father is alive, we include him in the list of 'other affected people'
	void markOtherAffectedPeople(const Population &population);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	Man *m_pFather;

	static double m_boyGirlRatio;
	static ProbabilityDistribution *m_pPregDurationDist;
};

#endif // EVENTBIRTH_H
