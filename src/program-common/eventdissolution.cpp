#include "eventdissolution.h"
#include "eventformation.h"
#include "hazardfunctionformationsimple.h"
#include "jsonconfig.h"
#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace std;

EventDissolution::EventDissolution(Person *pPerson1, Person *pPerson2, double formationTime) : SimpactEvent(pPerson1, pPerson2)
{
	m_formationTime = formationTime;
}

EventDissolution::~EventDissolution()
{
}

std::string EventDissolution::getDescription(double tNow) const
{
	char str[1024];

	sprintf(str, "Dissolution between %s and %s, relationship was formed at %g (%g ago)", 
		     getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str(), m_formationTime, tNow-m_formationTime);

	return std::string(str);
}

void EventDissolution::writeLogs(double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "dissolution", tNow, pPerson1, pPerson2);

	// Relationship log will be written when handling the dissolution in person.cpp, that way
	// it will also be handled when it's because someone dies
}

void EventDissolution::fire(State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	pPerson1->removeRelationship(pPerson2, t, false);
	pPerson2->removeRelationship(pPerson1, t, false);

	// A new formation event should only be scheduled if neither person is in the
	// final AIDS stage
	if (pPerson1->getInfectionStage() != Person::AIDSFinal && pPerson2->getInfectionStage() != Person::AIDSFinal)
	{
		// Need to add a new formation event for these two
		EventFormation *pFormationEvent = new EventFormation(pPerson1, pPerson2, t);
		population.onNewEvent(pFormationEvent);
	}
}

double EventDissolution::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	double tr = m_formationTime;
	double tMax = getTMax(pPerson1, pPerson2);

	HazardFunctionFormationSimple h0(pPerson1, pPerson2, tr, a0, a1, a2, a3, a4, a5, Dp, b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
	//return ExponentialHazardToInternalTime(pPerson1, pPerson2, t0, dt, tr, a0, a1, a2, a3, a4, a5, Dp, b, true, tMax);
}

double EventDissolution::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	double tr = m_formationTime;
	double tMax = getTMax(pPerson1, pPerson2);

	HazardFunctionFormationSimple h0(pPerson1, pPerson2, tr, a0, a1, a2, a3, a4, a5, Dp, b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
	//return ExponentialHazardToRealTime(pPerson1, pPerson2, t0, Tdiff, tr, a0, a1, a2, a3, a4, a5, Dp, b, true, tMax);
}

double EventDissolution::getTMax(Person *pPerson1, Person *pPerson2)
{
	assert(pPerson1 != 0 && pPerson2 != 0);

	double tb1 = pPerson1->getDateOfBirth();
	double tb2 = pPerson2->getDateOfBirth();

	double tMax = tb1;

	if (tb2 < tMax)
		tMax = tb2;

	assert(tMaxDiff > 0);
	tMax += tMaxDiff;

	return tMax;
}

double EventDissolution::a0   = 0;		// baseline_factor
double EventDissolution::a1   = 0;		// male_current_relations_factor
double EventDissolution::a2   = 0;		// female_current_relations_factor
double EventDissolution::a3   = 0;		// current_relations_difference_factor
double EventDissolution::a4   = 0;		// mean_age_factor
double EventDissolution::a5   = 0;		// age_difference_factor
double EventDissolution::Dp   = 0;		// preferred_age_difference
double EventDissolution::b    = 0;		// last_change_factor
double EventDissolution::tMaxDiff = 0;		// t_max

void EventDissolution::processConfig(ConfigSettings &config)
{
	if (!config.getKeyValue("dissolution.alpha_0", a0) ||
	    !config.getKeyValue("dissolution.alpha_1", a1) ||
	    !config.getKeyValue("dissolution.alpha_2", a2) ||
	    !config.getKeyValue("dissolution.alpha_3", a3) ||
	    !config.getKeyValue("dissolution.alpha_4", a4) ||
	    !config.getKeyValue("dissolution.alpha_5", a5) ||
	    !config.getKeyValue("dissolution.Dp", Dp) ||
	    !config.getKeyValue("dissolution.beta", b) ||
	    !config.getKeyValue("dissolution.t_max", tMaxDiff, 0) )
		abortWithMessage(config.getErrorString());
}

void EventDissolution::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("dissolution.alpha_0", a0) ||
	    !config.addKey("dissolution.alpha_1", a1) ||
	    !config.addKey("dissolution.alpha_2", a2) ||
	    !config.addKey("dissolution.alpha_3", a3) ||
	    !config.addKey("dissolution.alpha_4", a4) ||
	    !config.addKey("dissolution.alpha_5", a5) ||
	    !config.addKey("dissolution.Dp", Dp) ||
	    !config.addKey("dissolution.beta", b) ||
	    !config.addKey("dissolution.t_max", tMaxDiff) )
		abortWithMessage(config.getErrorString());
}

JSONConfig dissolutionJSONConfig(R"JSON(
        "EventDissolution": { 
            "depends": null,
            "params": [ 
                ["dissolution.alpha_0", 0.1],
                ["dissolution.alpha_1", 0],
                ["dissolution.alpha_2", 0],
                ["dissolution.alpha_3", 0],
                ["dissolution.alpha_4", 0],
                ["dissolution.alpha_5", 0],
                ["dissolution.Dp", 0],
                ["dissolution.beta", 0],
                ["dissolution.t_max", 200] ],
            "info": [ 
                "These are the parameters for the hazard in the dissolution event.",
                "see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html",
                "for more information."
            ]
        })JSON");

