#include "evthazardformationagegaprefyear.h"
#include "eventformation.h"
#include "eventdebut.h"
#include "configsettings.h"
#include "hazardfunctionformationagegaprefyear.h"
#include "jsonconfig.h"
#include <algorithm>

// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads
// WARNING: the same instance can be called from multiple threads

EvtHazardFormationAgeGapRefYear::EvtHazardFormationAgeGapRefYear(double a0, double a1, double a2, double a3, 
		           double a4, double a6,
			       double a7, double a8, double a10, double aDist,
				   double agfmConst, double agfmExp, double agfmAge,
				   double agfwConst, double agfwExp, double agfwAge,
				   double numRelScaleMan, double numRelScaleWoman,
				   double b, double tMax,
				   double maxAgeRefDiff)
{
	m_a0 = a0;
	m_a1 = a1;
	m_a2 = a2;
	m_a3 = a3;
	m_a4 = a4;
	m_a6 = a6;
	m_a7 = a7;
	m_a8 = a8;
	m_a10 = a10;
	m_aDist = aDist;

	m_agfmConst = agfmConst;
	m_agfmExp = agfmExp;
	m_agfmAge = agfmAge;
	m_agfwConst = agfwConst;
	m_agfwExp = agfwExp;
	m_agfwAge = agfwAge;

	m_numRelScaleMan = numRelScaleMan;
	m_numRelScaleWoman = numRelScaleWoman;

	m_b = b;
	m_tMax = tMax;
	m_tMaxAgeRefDiff = maxAgeRefDiff;
}

EvtHazardFormationAgeGapRefYear::~EvtHazardFormationAgeGapRefYear()
{
}

double EvtHazardFormationAgeGapRefYear::getTMax(Person *pPerson1, Person *pPerson2)
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

double EvtHazardFormationAgeGapRefYear::calculateInternalTimeInterval(const SimpactPopulation &population, 
	                                     const SimpactEvent &event, double t0, double dt)
{
	Person *pPerson1 = event.getPerson(0);
	Person *pPerson2 = event.getPerson(1);

	double tMax = getTMax(pPerson1, pPerson2);

	const EventFormation &eventFormation = static_cast<const EventFormation &>(event);
	double lastDissTime = eventFormation.getLastDissolutionTime();

	double a0 = getA0(population, pPerson1, pPerson2);
	double tr = getTr(population, pPerson1, pPerson2, t0, lastDissTime);
	double ageRefYear = population.getReferenceYear();

	if (t0 - ageRefYear < -1e-8)
		abortWithMessage("EvtHazardFormationAgeGapRefYear: t0 is smaller than ageRefYear (1)");
	if (t0 - ageRefYear > m_tMaxAgeRefDiff+1e-8)
		abortWithMessage("EvtHazardFormationAgeGapRefYear: t0 - ageRefYear exceeds maximum specified difference (1)");

	// Note: we need to use a0 here, not m_a0
	HazardFunctionFormationAgeGapRefYear h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a8, m_a10, 
			                                m_agfmConst, m_agfmExp, m_agfmAge, m_agfwConst, m_agfwExp, m_agfwAge,
											m_numRelScaleMan, m_numRelScaleWoman,
											m_b, ageRefYear);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EvtHazardFormationAgeGapRefYear::solveForRealTimeInterval(const SimpactPopulation &population,
			                const SimpactEvent &event, double Tdiff, double t0)
{
	Person *pPerson1 = event.getPerson(0);
	Person *pPerson2 = event.getPerson(1);

	double tMax = getTMax(pPerson1, pPerson2);

	const EventFormation &eventFormation = static_cast<const EventFormation &>(event);
	double lastDissTime = eventFormation.getLastDissolutionTime();

	double a0 = getA0(population, pPerson1, pPerson2);
	double tr = getTr(population, pPerson1, pPerson2, t0, lastDissTime);
	double ageRefYear = population.getReferenceYear();

	if (t0 - ageRefYear < -1e-8)
		abortWithMessage("EvtHazardFormationAgeGapRefYear: t0 is smaller than ageRefYear (2)");
	if (t0 - ageRefYear > m_tMaxAgeRefDiff+1e-8)
		abortWithMessage("EvtHazardFormationAgeGapRefYear: t0 - ageRefYear exceeds maximum specified difference (2)");

	// Note: we need to use a0 here, not m_a0
	HazardFunctionFormationAgeGapRefYear h0(pPerson1, pPerson2, tr, a0, m_a1, m_a2, m_a3, m_a4, m_a8, m_a10, 
			                                m_agfmConst, m_agfmExp, m_agfmAge, m_agfwConst, m_agfwExp, m_agfwAge,
											m_numRelScaleMan, m_numRelScaleWoman,
											m_b, ageRefYear);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EvtHazardFormationAgeGapRefYear::getA0(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2)
{
	double lastPopSizeTime = 0;
	double n = population.getLastKnownPopulationSize(lastPopSizeTime);
	double a0i = pPerson1->getFormationEagernessParameter();
	double a0j = pPerson2->getFormationEagernessParameter();
	double a0_base = m_a0 + (a0i + a0j)*m_a6 + std::abs(a0i-a0j)*m_a7;

	a0_base += m_aDist * pPerson1->getDistanceTo(pPerson2);

	double eyeCapsFraction = population.getEyeCapsFraction();
	// reduces to old code if eyeCapsFraction == 1
	double a0_total = a0_base - std::log((n/2.0)*eyeCapsFraction); // log(x/(n/2)) = log(x) - log(n/2) = a0_base - log(n/2)
	
	return a0_total;
}

double EvtHazardFormationAgeGapRefYear::getTr(const SimpactPopulation &population, Person *pPerson1, Person *pPerson2, double t0, double lastDissTime)
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

EvtHazard *EvtHazardFormationAgeGapRefYear::processConfig(ConfigSettings &config)
{
	double a0, a1, a2, a3, a4, a6, a7, a8, a10, aDist, b, tMax, tMaxAgeRefDiff;
	double agfmConst, agfmExp, agfmAge, agfwConst, agfwExp, agfwAge;
	double numRelScaleMan, numRelScaleWoman;
	bool_t r;

	if (!(r = config.getKeyValue("formation.hazard.agegapry.baseline", a0)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.numrel_man", a1)) ||
		!(r = config.getKeyValue("formation.hazard.agegapry.numrel_scale_man", numRelScaleMan)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.numrel_woman", a2)) ||
		!(r = config.getKeyValue("formation.hazard.agegapry.numrel_scale_woman", numRelScaleWoman)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.numrel_diff", a3)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.meanage", a4)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.eagerness_sum", a6)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.eagerness_diff", a7)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_agescale_man", a8)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_agescale_woman", a10)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_man_const", agfmConst)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_man_exp", agfmExp)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_man_age", agfmAge)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_woman_const", agfwConst)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_woman_exp", agfwExp)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.gap_factor_woman_age", agfwAge)) ||
		!(r = config.getKeyValue("formation.hazard.agegapry.distance", aDist)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.beta", b)) ||
	    !(r = config.getKeyValue("formation.hazard.agegapry.t_max", tMax, 0)) ||
		!(r = config.getKeyValue("formation.hazard.agegapry.maxageref.diff", tMaxAgeRefDiff, 0))
		)
		abortWithMessage(r.getErrorString());
	
	return new EvtHazardFormationAgeGapRefYear(a0,a1,a2,a3,a4,a6,a7,a8,a10,aDist,
	                                           agfmConst, agfmExp, agfmAge, agfwConst, agfwExp, agfwAge,
											   numRelScaleMan, numRelScaleWoman,
			                                   b,tMax,tMaxAgeRefDiff);
}

void EvtHazardFormationAgeGapRefYear::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("formation.hazard.type", "agegapry")) ||
	    !(r = config.addKey("formation.hazard.agegapry.baseline", m_a0)) ||
	    !(r = config.addKey("formation.hazard.agegapry.numrel_man", m_a1)) ||
		!(r = config.addKey("formation.hazard.agegapry.numrel_scale_man", m_numRelScaleMan)) ||
	    !(r = config.addKey("formation.hazard.agegapry.numrel_woman", m_a2)) ||
		!(r = config.addKey("formation.hazard.agegapry.numrel_scale_woman", m_numRelScaleWoman)) ||
	    !(r = config.addKey("formation.hazard.agegapry.numrel_diff", m_a3)) ||
	    !(r = config.addKey("formation.hazard.agegapry.meanage", m_a4)) ||
	    !(r = config.addKey("formation.hazard.agegapry.eagerness_sum", m_a6)) ||
	    !(r = config.addKey("formation.hazard.agegapry.eagerness_diff", m_a7)) ||
	    !(r = config.addKey("formation.hazard.agegapry.gap_agescale_man", m_a8)) ||
	    !(r = config.addKey("formation.hazard.agegapry.gap_agescale_woman", m_a10)) ||
	    !(r = config.addKey("formation.hazard.agegapry.gap_factor_man_const", m_agfmConst)) ||
		!(r = config.addKey("formation.hazard.agegapry.gap_factor_man_exp", m_agfmExp)) ||
		!(r = config.addKey("formation.hazard.agegapry.gap_factor_man_age", m_agfmAge)) ||
	    !(r = config.addKey("formation.hazard.agegapry.gap_factor_woman_const", m_agfwConst)) ||
		!(r = config.addKey("formation.hazard.agegapry.gap_factor_woman_exp", m_agfwExp)) ||
		!(r = config.addKey("formation.hazard.agegapry.gap_factor_woman_age", m_agfwAge)) ||
		!(r = config.addKey("formation.hazard.agegapry.distance", m_aDist)) ||
	    !(r = config.addKey("formation.hazard.agegapry.beta", m_b)) ||
	    !(r = config.addKey("formation.hazard.agegapry.t_max", m_tMax)) ||
		!(r = config.addKey("formation.hazard.agegapry.maxageref.diff", m_tMaxAgeRefDiff))
		)
		abortWithMessage(r.getErrorString());
}

JSONConfig agegapRefYearFormationJSONConfig(R"JSON(
        "EventFormation_agegap_refyear": { 
            "depends": [ "EventFormationTypes", "formation.hazard.type", "agegapry" ],
            "params": [ 
                ["formation.hazard.agegapry.baseline", 0.1],
                ["formation.hazard.agegapry.numrel_man", 0],
				["formation.hazard.agegapry.numrel_scale_man", 0],
                ["formation.hazard.agegapry.numrel_woman", 0],
				["formation.hazard.agegapry.numrel_scale_woman", 0],
                ["formation.hazard.agegapry.numrel_diff", 0],
                ["formation.hazard.agegapry.meanage", 0],
                ["formation.hazard.agegapry.eagerness_sum", 0],
                ["formation.hazard.agegapry.eagerness_diff", 0],
                ["formation.hazard.agegapry.gap_factor_man_const", 0],
				["formation.hazard.agegapry.gap_factor_man_exp", 0],
				["formation.hazard.agegapry.gap_factor_man_age", 0],
                ["formation.hazard.agegapry.gap_agescale_man", 0],
                ["formation.hazard.agegapry.gap_factor_woman_const", 0],
				["formation.hazard.agegapry.gap_factor_woman_exp", 0],
				["formation.hazard.agegapry.gap_factor_woman_age", 0],
                ["formation.hazard.agegapry.gap_agescale_woman", 0],
				["formation.hazard.agegapry.distance", 0],
                ["formation.hazard.agegapry.beta", 0],
                ["formation.hazard.agegapry.t_max", 200],
				["formation.hazard.agegapry.maxageref.diff", 1]
			],
            "info": [ 
                "These are the parameters for the hazard in the 'agegapry' formation event.",
                "see http://research.edm.uhasselt.be/jori/simpact/",
                "for more information."
            ]
        })JSON");

