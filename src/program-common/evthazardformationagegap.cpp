#include "evthazardformationagegap.h"
#include "eventformation.h"
#include "eventdebut.h"
#include "configsettings.h"
#include "hazardfunctionformationagegap.h"
#include "jsonconfig.h"
#include <algorithm>

// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads

EvtHazardFormationAgeGap::EvtHazardFormationAgeGap(double a0, double a1, double a2, double a3, double a4, double a5, double a6,
			       double a7, double a8, double a9, double a10, double b, double tMax)
{
	m_a0 = a0;
	m_a1 = a1;
	m_a2 = a2;
	m_a3 = a3;
	m_a4 = a4;
	m_a5 = a5;
	m_a6 = a6;
	m_a7 = a7;
	m_a8 = a8;
	m_a9 = a9;
	m_a10 = a10;
	m_b = b;
	m_tMax = tMax;
}

EvtHazardFormationAgeGap::~EvtHazardFormationAgeGap()
{
}

double EvtHazardFormationAgeGap::getTMax(Person *pPerson1, Person *pPerson2)
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

double EvtHazardFormationAgeGap::calculateInternalTimeInterval(const SimpactPopulation &population, 
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
	HazardFunctionFormationAgeGap h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_a8, m_a9, m_a10, m_b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EvtHazardFormationAgeGap::solveForRealTimeInterval(const SimpactPopulation &population,
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
	HazardFunctionFormationAgeGap h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_a8, m_a9, m_a10, m_b);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EvtHazardFormationAgeGap::getA0(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2)
{
	double lastPopSizeTime = 0;
	double n = population.getLastKnownPopulationSize(lastPopSizeTime);
	double a0i = pPerson1->getFormationEagernessParameter();
	double a0j = pPerson2->getFormationEagernessParameter();
	double a0_base = m_a0 + (a0i + a0j)*m_a6 + std::abs(a0i-a0j)*m_a7;

	double eyeCapsFraction = population.getEyeCapsFraction();
	// reduces to old code if eyeCapsFraction == 1
	double a0_total = a0_base - std::log((n/2.0)*eyeCapsFraction); // log(x/(n/2)) = log(x) - log(n/2) = a0_base - log(n/2)
	
	return a0_total;
}

double EvtHazardFormationAgeGap::getTr(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2, double t0, double lastDissTime)
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

EvtHazard *EvtHazardFormationAgeGap::processConfig(ConfigSettings &config)
{
	double a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, b, tMax;

	if (!config.getKeyValue("formation.hazard.agegap.baseline", a0) ||
	    !config.getKeyValue("formation.hazard.agegap.numrel_man", a1) ||
	    !config.getKeyValue("formation.hazard.agegap.numrel_woman", a2) ||
	    !config.getKeyValue("formation.hazard.agegap.numrel_diff", a3) ||
	    !config.getKeyValue("formation.hazard.agegap.meanage", a4) ||
	    !config.getKeyValue("formation.hazard.agegap.gap_factor_man", a5) ||
	    !config.getKeyValue("formation.hazard.agegap.eagerness_sum", a6) ||
	    !config.getKeyValue("formation.hazard.agegap.eagerness_diff", a7) ||
	    !config.getKeyValue("formation.hazard.agegap.gap_agescale_man", a8) ||
	    !config.getKeyValue("formation.hazard.agegap.gap_factor_woman", a9) ||
	    !config.getKeyValue("formation.hazard.agegap.gap_agescale_woman", a10) ||
	    !config.getKeyValue("formation.hazard.agegap.beta", b) ||
	    !config.getKeyValue("formation.hazard.agegap.t_max", tMax, 0) )
		abortWithMessage(config.getErrorString());
	
	return new EvtHazardFormationAgeGap(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,b,tMax);
}

void EvtHazardFormationAgeGap::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("formation.hazard.type", "agegap") ||
	    !config.addKey("formation.hazard.agegap.baseline", m_a0) ||
	    !config.addKey("formation.hazard.agegap.numrel_man", m_a1) ||
	    !config.addKey("formation.hazard.agegap.numrel_woman", m_a2) ||
	    !config.addKey("formation.hazard.agegap.numrel_diff", m_a3) ||
	    !config.addKey("formation.hazard.agegap.meanage", m_a4) ||
	    !config.addKey("formation.hazard.agegap.gap_factor_man", m_a5) ||
	    !config.addKey("formation.hazard.agegap.eagerness_sum", m_a6) ||
	    !config.addKey("formation.hazard.agegap.eagerness_diff", m_a7) ||
	    !config.addKey("formation.hazard.agegap.gap_agescale_man", m_a8) ||
	    !config.addKey("formation.hazard.agegap.gap_factor_woman", m_a9) ||
	    !config.addKey("formation.hazard.agegap.gap_agescale_woman", m_a10) ||
	    !config.addKey("formation.hazard.agegap.beta", m_b) ||
	    !config.addKey("formation.hazard.agegap.t_max", m_tMax) )
		abortWithMessage(config.getErrorString());
}

JSONConfig agegapFormationJSONConfig(R"JSON(
        "EventFormation_agegap": { 
            "depends": [ "EventFormationTypes", "formation.hazard.type", "agegap" ],
            "params": [ 
                ["formation.hazard.agegap.baseline", 0.1],
                ["formation.hazard.agegap.numrel_man", 0],
                ["formation.hazard.agegap.numrel_woman", 0],
                ["formation.hazard.agegap.numrel_diff", 0],
                ["formation.hazard.agegap.meanage", 0],
                ["formation.hazard.agegap.eagerness_sum", 0],
                ["formation.hazard.agegap.eagerness_diff", 0],
                ["formation.hazard.agegap.gap_factor_man", 0],
                ["formation.hazard.agegap.gap_agescale_man", 0],
                ["formation.hazard.agegap.gap_factor_woman", 0],
                ["formation.hazard.agegap.gap_agescale_woman", 0],
                ["formation.hazard.agegap.beta", 0],
                ["formation.hazard.agegap.t_max", 200] ],
            "info": [ 
                "These are the parameters for the hazard in the 'agegap' formation event.",
                "see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html",
                "for more information."
            ]
        })JSON");

