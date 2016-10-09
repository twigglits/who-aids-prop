#include "eventtest.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventdropout.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"

#include <iostream>

using namespace std;

EventTest::EventTest(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventTest::~EventTest()
{
}

string EventTest::getDescription(double tNow)
{
	char str[1024];

	sprintf(str, "Test event for %s", getPerson(0)->getName().c_str());
	return string(str);
}

void EventTest::writeLogs(double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(false, "hivtest", tNow, pPerson, 0);

	LogEvent.print(",CD4,%g", pPerson->getCD4Count(tNow));
}

bool EventTest::isEligibleForTreatment(double t)
{
	Person *pPerson = getPerson(0);
	assert(s_cd4Threshold >= 0);

	if (pPerson->getNumberTreatmentStarted() > 0) // if the person has already received treatment, (s)he's still eligible
		return true;

	// Check the threshold
	if (pPerson->getCD4Count(t) < s_cd4Threshold)
		return true;

	return false;
}

void EventTest::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson->isInfected());
	assert(!pPerson->hasLoweredViralLoad());
	assert(s_treatmentVLLogFrac >= 0 && s_treatmentVLLogFrac <= 1.0);

	if (isEligibleForTreatment(t))
	{
		SimpactEvent::writeEventLogStart(true, "(treatment)", t, pPerson, 0);

		// Person is starting treatment, no further HIV test events will follow
		pPerson->lowerViralLoad(s_treatmentVLLogFrac, t);

		// Dropout event becomes possible
		EventDropout *pEvtDropout = new EventDropout(pPerson, t);
		population.onNewEvent(pEvtDropout);
	}
	else
	{
		// Schedule a new test event
		EventTest *pNewTest = new EventTest(pPerson);
		population.onNewEvent(pNewTest);
	}
}

double EventTest::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	// TODO: this is just a temporaty solution, until a real hazard has been defined
	
	int count = 0;
	int maxCount = 1024;
	double dt = -1;

	assert(s_pTestingDistribution);

	while (dt < 0 && count++ < maxCount)
		dt = s_pTestingDistribution->pickNumber();

	if (dt < 0)
		abortWithMessage("EventTest: couldn't find a positive time interval for next event");

	return dt;
}

double EventTest::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO: this will need to change when using a real hazard
	return dt;
}

double EventTest::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO: this will need to change when using a real hazard
	return Tdiff;
}

double EventTest::s_treatmentVLLogFrac = -1;
double EventTest::s_cd4Threshold = -1;

ProbabilityDistribution *EventTest::s_pTestingDistribution = 0;

void EventTest::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (!config.getKeyValue("hivtest.cd4.threshold", s_cd4Threshold, 0) ||
	    !config.getKeyValue("hivtest.fraction.log_viralload", s_treatmentVLLogFrac, 0, 1))
		abortWithMessage(config.getErrorString());

	delete s_pTestingDistribution;
	s_pTestingDistribution = getDistributionFromConfig(config, pRndGen, "hivtest.interval");
}

void EventTest::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("hivtest.cd4.threshold", s_cd4Threshold) ||
	    !config.addKey("hivtest.fraction.log_viralload", s_treatmentVLLogFrac))
		abortWithMessage(config.getErrorString());

	addDistributionToConfig(s_pTestingDistribution, config, "hivtest.interval");
}

