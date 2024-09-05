#ifndef EVENTDIAGNOSIS_H

#define EVENTDIAGNOSIS_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class HazardFunctionDiagnosis : public HazardFunctionExp
{
public:
	HazardFunctionDiagnosis(Person *pPerson, double baseline, double ageFactor, double genderFactor, double diagPartnersFactor, double isDiagnosedFactor, double beta, double HSV2factor, double eagernessFactor, double pregnancyFactor, double AGYWFactor); 
	double evaluate(double t);
private:
	Person *m_pPerson;
	const double m_baseline, m_ageFactor, m_genderFactor, m_diagPartnersFactor;
	const double m_isDiagnosedFactor, m_beta, m_HSV2factor, m_eagernessFactor, m_pregnancyFactor, m_AGYWFactor;
};

class EventDiagnosis : public SimpactEvent
{
public:
	EventDiagnosis(Person *pPerson);
	~EventDiagnosis();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	// static bool getY(const Person *pPerson, const State *pState);
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	void markOtherAffectedPeople(const PopulationStateInterface &population);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	static double getTMax(const Person *pPerson);

	// bool getY(double t, const State *pState);  
	static double s_baseline;
	static double s_ageFactor;
	static double s_genderFactor;
	static double s_diagPartnersFactor;
	static double s_isDiagnosedFactor;
	static double s_beta;
	static double s_tMax;
	static double s_HSV2factor; 
	static double s_eagernessFactor;
	static double s_pregnancyFactor;
	static double s_AGYWFactor;
};

#endif // EVENTDIAGNOSIS_H

