#include "eventmonitoring.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventdropout.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "piecewiselinearfunction.h"
#include "point2d.h"

#include <iostream>

using namespace std;

EventMonitoring::EventMonitoring(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson)
{
	m_scheduleImmediately = scheduleImmediately;
}

EventMonitoring::~EventMonitoring()
{
}

string EventMonitoring::getDescription(double tNow)
{
	char str[1024];

	sprintf(str, "Monitoring event for %s", getPerson(0)->getName().c_str());
	return string(str);
}

void EventMonitoring::writeLogs(double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(false, "monitoring", tNow, pPerson, 0);

	LogEvent.print(",CD4,%g", pPerson->getCD4Count(tNow));
}

bool EventMonitoring::isEligibleForTreatment(double t)
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

bool EventMonitoring::isWillingToStartTreatment(double t, GslRandomNumberGenerator *pRndGen)
{
	Person *pPerson = getPerson(0);

	// Coin toss
	double x = pRndGen->pickRandomDouble();
	if (x < pPerson->getARTAcceptanceThreshold())
		return true;

	return false;
}

void EventMonitoring::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pPerson = getPerson(0);

	assert(pPerson->isInfected());
	assert(!pPerson->hasLoweredViralLoad());
	assert(s_treatmentVLLogFrac >= 0 && s_treatmentVLLogFrac <= 1.0);

	if (isEligibleForTreatment(t) && isWillingToStartTreatment(t, pRndGen))
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
		// Schedule a new monitoring event
		EventMonitoring *pNewMonitor = new EventMonitoring(pPerson);
		population.onNewEvent(pNewMonitor);
	}
}

double EventMonitoring::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	// This is for the monitoring event that should be scheduled right after the
	// diagnosis event
	if (m_scheduleImmediately)
	{
		double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
		return hour * pRndGen->pickRandomDouble();
	}

	assert(s_pRecheckInterval);

	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);
	double currentTime = population.getTime();
	double cd4 = pPerson->getCD4Count(currentTime);
	double dt = s_pRecheckInterval->evaluate(cd4);

	assert(dt >= 0);
	return dt;
}

double EventMonitoring::s_treatmentVLLogFrac = -1;
double EventMonitoring::s_cd4Threshold = -1;
PieceWiseLinearFunction *EventMonitoring::s_pRecheckInterval = 0;

void EventMonitoring::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	if (!config.getKeyValue("monitoring.cd4.threshold", s_cd4Threshold, 0) ||
	    !config.getKeyValue("monitoring.fraction.log_viralload", s_treatmentVLLogFrac, 0, 1))
		abortWithMessage(config.getErrorString());

	vector<double> intervalX, intervalY;
	double leftValue, rightValue;

	if (!config.getKeyValue("monitoring.interval.piecewise.cd4s", intervalX) ||
	    !config.getKeyValue("monitoring.interval.piecewise.times", intervalY) ||
	    !config.getKeyValue("monitoring.interval.piecewise.left", leftValue) ||
	    !config.getKeyValue("monitoring.interval.piecewise.right", rightValue))
		abortWithMessage(config.getErrorString());

	for (size_t i = 0 ; i < intervalX.size()-1 ; i++)
	{
		if (intervalX[i+1] < intervalX[i])
			abortWithMessage("CD4 values must be increasing in 'monitoring.interval.piecewise.cd4s'");
	}

	if (intervalX.size() < 1)
		abortWithMessage("CD4 value list 'monitoring.interval.piecewise.cd4s' must contain at least one element");

	if (intervalX.size() != intervalY.size())
		abortWithMessage("Lists 'monitoring.interval.piecewise.cd4s' and 'monitoring.interval.piecewise.times' must contain the same number of elements");

	vector<Point2D> points;

	for (size_t i = 0 ; i < intervalX.size() ; i++)
		points.push_back(Point2D(intervalX[i], intervalY[i]));

	delete s_pRecheckInterval;
	s_pRecheckInterval = new PieceWiseLinearFunction(points, leftValue, rightValue);
}

void EventMonitoring::obtainConfig(ConfigWriter &config)
{
	assert(s_pRecheckInterval);

	const vector<Point2D> &points = s_pRecheckInterval->getPoints();
	vector<double> intervalX, intervalY;

	for (size_t i = 0 ; i < points.size() ; i++)
	{
		intervalX.push_back(points[i].x);
		intervalY.push_back(points[i].y);
	}

	if (!config.addKey("monitoring.cd4.threshold", s_cd4Threshold) ||
	    !config.addKey("monitoring.fraction.log_viralload", s_treatmentVLLogFrac) ||
	    !config.addKey("monitoring.interval.piecewise.cd4s", intervalX) ||
	    !config.addKey("monitoring.interval.piecewise.times", intervalY) ||
	    !config.addKey("monitoring.interval.piecewise.left", s_pRecheckInterval->getLeftValue()) ||
	    !config.addKey("monitoring.interval.piecewise.right", s_pRecheckInterval->getRightValue()) )
		abortWithMessage(config.getErrorString());
}

