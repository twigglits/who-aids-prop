#include "evthazardformationsimple.h"
#include "eventformation.h"
#include "eventdebut.h"
#include "configsettings.h"
#include "hazardfunctionformationsimple.h"
#include "jsonconfig.h"
#include <algorithm>

// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads

EvtHazardFormationSimple::EvtHazardFormationSimple(double a0, double a1, double a2, double a3, double a4, double a5, double a6,
			       double a7, double Dp, double b, double tMax)
{
	m_a0 = a0;
	m_a1 = a1;
	m_a2 = a2;
	m_a3 = a3;
	m_a4 = a4;
	m_a5 = a5;
	m_a6 = a6;
	m_a7 = a7;
	m_Dp = Dp;
	m_b = b;
	m_tMax = tMax;
}

EvtHazardFormationSimple::~EvtHazardFormationSimple()
{
}

double EvtHazardFormationSimple::getTMax(Person *pPerson1, Person *pPerson2)
{
	assert(pPerson1 != 0 && pPerson2 != 0);

	double tb1 = pPerson1->getDateOfBirth();
	double tb2 = pPerson2->getDateOfBirth();

	double tMax = tb1;

	if (tb2 < tMax)
		tMax = tb2;

	assert(m_tMax > 0);
	tMax += m_tMax;

	return tMax;
}

double EvtHazardFormationSimple::calculateInternalTimeInterval(const SimpactPopulation &population, 
	                                     const SimpactEvent &event, double t0, double dt)
{
	Person *pPerson1 = event.getPerson(0);
	Person *pPerson2 = event.getPerson(1);

	double tMax = getTMax(pPerson1, pPerson2);

	const EventFormation &eventFormation = static_cast<const EventFormation &>(event);
	double lastDissTime = eventFormation.getLastDissolutionTime();

	double a0 = getA0(population, pPerson1, pPerson2);
	double tr = getTr(population, pPerson1, pPerson2, t0, lastDissTime);

	// Note: we need to use a0 here, not m_a0
	HazardFunctionFormationSimple h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_Dp, m_b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);

	//return ExponentialHazardToInternalTime(pPerson1, pPerson2, t0, dt, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_Dp, m_b, true, tMax);
}

double EvtHazardFormationSimple::solveForRealTimeInterval(const SimpactPopulation &population,
			                const SimpactEvent &event, double Tdiff, double t0)
{
	Person *pPerson1 = event.getPerson(0);
	Person *pPerson2 = event.getPerson(1);

	double tMax = getTMax(pPerson1, pPerson2);

	const EventFormation &eventFormation = static_cast<const EventFormation &>(event);
	double lastDissTime = eventFormation.getLastDissolutionTime();

	double a0 = getA0(population, pPerson1, pPerson2);
	double tr = getTr(population, pPerson1, pPerson2, t0, lastDissTime);

	// Note: we need to use a0 here, not m_a0
	HazardFunctionFormationSimple h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_Dp, m_b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
	//return ExponentialHazardToRealTime(pPerson1, pPerson2, t0, Tdiff, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_Dp, m_b, true, tMax);
}

double EvtHazardFormationSimple::getA0(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2)
{
	double lastKnownPopSizeTime = 0;
	double n = population.getLastKnownPopulationSize(lastKnownPopSizeTime);
	double a0i = pPerson1->getFormationEagernessParameter();
	double a0j = pPerson2->getFormationEagernessParameter();
	double a0_base = m_a0 + (a0i + a0j)*m_a6 * std::abs(a0i-a0j)*m_a7;

	double eyeCapsFraction = population.getEyeCapsFraction();
	// reduces to old code if eyeCapsFraction == 1
	double a0_total = a0_base - std::log((n/2.0)*eyeCapsFraction); // log(x/(n/2)) = log(x) - log(n/2) = a0_base - log(n/2)
	
	return a0_total;
}

double EvtHazardFormationSimple::getTr(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2, double t0, double lastDissTime)
{
	double tr = lastDissTime;
	double tBi = pPerson1->getDateOfBirth();
	double tBj = pPerson2->getDateOfBirth();
	
	if (tr < 0) // did not have a relationship before, use 
	{
		// get time at which both persons became 15 years (or in general, the debut age) old
		double t1 = tBi+EventDebut::getDebutAge();
		double t2 = tBj+EventDebut::getDebutAge();

		tr = std::max(t1,t2);

		assert(t0-tr > -1e-10); // something slightly negative is possible due to finite precision and error accumulation
	}

	return tr;
}

EvtHazard *EvtHazardFormationSimple::processConfig(ConfigSettings &config)
{
	double a0, a1, a2, a3, a4, a5, a6, a7, Dp, b, tMax;

	if (!config.getKeyValue("formation.hazard.simple.alpha_0", a0) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_1", a1) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_2", a2) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_3", a3) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_4", a4) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_5", a5) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_6", a6) ||
	    !config.getKeyValue("formation.hazard.simple.alpha_7", a7) ||
	    !config.getKeyValue("formation.hazard.simple.Dp", Dp) ||
	    !config.getKeyValue("formation.hazard.simple.beta", b) ||
	    !config.getKeyValue("formation.hazard.simple.t_max", tMax, 0) )
		abortWithMessage(config.getErrorString());
	
	return new EvtHazardFormationSimple(a0,a1,a2,a3,a4,a5,a6,a7,Dp,b,tMax);
}

void EvtHazardFormationSimple::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("formation.hazard.type", "simple") ||
	    !config.addKey("formation.hazard.simple.alpha_0", m_a0) ||
	    !config.addKey("formation.hazard.simple.alpha_1", m_a1) ||
	    !config.addKey("formation.hazard.simple.alpha_2", m_a2) ||
	    !config.addKey("formation.hazard.simple.alpha_3", m_a3) ||
	    !config.addKey("formation.hazard.simple.alpha_4", m_a4) ||
	    !config.addKey("formation.hazard.simple.alpha_5", m_a5) ||
	    !config.addKey("formation.hazard.simple.alpha_6", m_a6) ||
	    !config.addKey("formation.hazard.simple.alpha_7", m_a7) ||
	    !config.addKey("formation.hazard.simple.Dp", m_Dp) ||
	    !config.addKey("formation.hazard.simple.beta", m_b) ||
	    !config.addKey("formation.hazard.simple.t_max", m_tMax) )
		abortWithMessage(config.getErrorString());
}

JSONConfig simpleFormationJSONConfig(R"JSON(
        "EventFormation_simple": { 
            "depends": ["EventFormationTypes", "formation.hazard.type", "simple"],
            "params": [ 
                ["formation.hazard.simple.alpha_0", 0.1],
                ["formation.hazard.simple.alpha_1", 0],
                ["formation.hazard.simple.alpha_2", 0],
                ["formation.hazard.simple.alpha_3", 0],
                ["formation.hazard.simple.alpha_4", 0],
                ["formation.hazard.simple.alpha_5", 0],
                ["formation.hazard.simple.alpha_6", 0],
                ["formation.hazard.simple.alpha_7", 0],
                ["formation.hazard.simple.Dp", 0],
                ["formation.hazard.simple.beta", 0],
                ["formation.hazard.simple.t_max", 200] ],
            "info": [
                "These are the parameters for the hazard in the simple formation event.",
                "see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html",
                "for more information."
            ]
        })JSON");

