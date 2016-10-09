#include "eventtreatment.h"
#include "eventaidsmortality.h"
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

EventTreatment::EventTreatment(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventTreatment::~EventTreatment()
{
}

double EventTreatment::treatmentTimeFrac = -1;
double EventTreatment::treatmentVLLogFrac = -1;
bool EventTreatment::treatmentEnabled = true;

double EventTreatment::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	Person *pPerson = getPerson(0);
	assert(pPerson->isInfected());
	assert(!pPerson->hasLoweredViralLoad()); // make sure we didn't have treatment yet
	
	// Check that the time this event gets scheduled is the same as the time this person
	// became infected, otherwise just using tSurv below is probably not ok
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	double t0 = population.getTime();
	assert(t0 == pPerson->getInfectionTime());

	double tSurv = EventAIDSMortality::getExpectedSurvivalTime(pPerson);
	assert(tSurv > 0);

	assert(treatmentTimeFrac >= 0 && treatmentTimeFrac <= 1.0);

	return tSurv * treatmentTimeFrac;
}

std::string EventTreatment::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	char str[1024];

	sprintf(str, "Treatment event for %s", pPerson->getName().c_str());
	return std::string(str);
}

void EventTreatment::writeLogs(double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "treatment", tNow, pPerson, 0);
}

void EventTreatment::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);

	Person *pPerson = getPerson(0);
	assert(pPerson->isInfected());
	assert(!pPerson->hasLoweredViralLoad());
	assert(treatmentVLLogFrac >= 0 && treatmentVLLogFrac <= 1.0);
	
	pPerson->lowerViralLoad(treatmentVLLogFrac, t);
}

void EventTreatment::processConfig(ConfigSettings &config)
{
	vector<string> allowedValues;
	string enableStr;

	allowedValues.push_back("yes");
	allowedValues.push_back("no");

	if (!config.getKeyValue("treatment.fraction.time", treatmentTimeFrac, 0, 1) ||
	    !config.getKeyValue("treatment.fraction.log_viralload", treatmentVLLogFrac, 0, 1) ||
	    !config.getKeyValue("treatment.enabled", enableStr, allowedValues) )
		abortWithMessage(config.getErrorString());

	if (enableStr == "yes")
		treatmentEnabled = true;
	else
		treatmentEnabled = false;
}

void EventTreatment::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("treatment.fraction.time", treatmentTimeFrac) ||
	    !config.addKey("treatment.fraction.log_viralload", treatmentVLLogFrac) ||
	    !config.addKey("treatment.enabled", treatmentEnabled) )
		abortWithMessage(config.getErrorString());

}


