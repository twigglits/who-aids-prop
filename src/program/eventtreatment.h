#ifndef EVENTTREATMENT_H

#define EVENTTREATMENT_H

#include "simpactevent.h"

class ConfigSettings;

class EventTreatment : public SimpactEvent
{
public:
	EventTreatment(Person *pPerson);
	~EventTreatment();

	std::string getDescription(double tNow) const;
	void writeLogs(double tNow) const;

	void fire(State *pState, double t);

	static void processConfig(ConfigSettings &config);
	static void obtainConfig(ConfigWriter &config);

	static bool isTreatmentEnabled()							{ return treatmentEnabled; }
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static double treatmentTimeFrac;
	static double treatmentVLLogFrac;
	static bool treatmentEnabled;
};

#endif // EVENTTREATMENT_H
