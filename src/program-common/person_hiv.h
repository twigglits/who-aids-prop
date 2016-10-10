#ifndef PERSON_HIV_H

#define PERSON_HIV_H

#include "aidstodutil.h"
#include "util.h"

class Person;
class ProbabilityDistribution;
class VspModel;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_HIV
{
public:
	enum InfectionType { None, Partner, Mother, Seed };
	enum InfectionStage { NoInfection, Acute, Chronic, AIDS, AIDSFinal };

	Person_HIV(Person *pSelf);
	~Person_HIV();

	InfectionType getInfectionType() const											{ return m_infectionType; }
	void setInfected(double t, Person *pOrigin, InfectionType iType);
	bool isInfected() const															{ if (m_infectionStage == NoInfection) return false; return true; }
	double getInfectionTime() const													{ assert(isInfected()); return m_infectionTime; }
	Person *getInfectionOrigin() const												{ assert(isInfected()); return m_pInfectionOrigin; }
	InfectionStage getInfectionStage() const										{ return m_infectionStage; }
	void setInChronicStage()														{ assert(m_infectionStage == Acute); m_infectionStage = Chronic; }
	void setInAIDSStage()															{ assert(m_infectionStage == Chronic); m_infectionStage = AIDS; }
	void setInFinalAIDSStage()														{ assert(m_infectionStage == AIDS); m_infectionStage = AIDSFinal; }
	double getAIDSMortalityTime() const												{ return m_aidsTodUtil.getTimeOfDeath(); }

	bool isDiagnosed() const														{ return (m_diagnoseCount > 0); }
	void increaseDiagnoseCount()													{ m_diagnoseCount++; }
	int getDiagnoseCount() const													{ return m_diagnoseCount; }

	double getSetPointViralLoad() const												{ assert(m_infectionStage != NoInfection); return m_Vsp; }
	double getOriginalViralLoad() const												{ assert(isInfected()); assert(m_VspOriginal > 0); return m_VspOriginal; }
	double getViralLoad() const;
	void lowerViralLoad(double fractionOnLogscale, double treatmentTime);
	bool hasLoweredViralLoad() const												{ assert(isInfected()); assert(m_Vsp > 0); return m_VspLowered; }
	double getLastTreatmentStartTime() const										{ assert(isInfected()); assert(m_Vsp > 0); assert(m_VspLowered); assert(m_lastTreatmentStartTime >= 0); return m_lastTreatmentStartTime; }
	void resetViralLoad(double dropoutTime);
	int getNumberTreatmentStarted() const											{ assert(isInfected()); return m_treatmentCount; }

	double getCD4Count(double t) const;
	double getARTAcceptanceThreshold() const										{ return m_artAcceptanceThreshold; }

	void markAIDSDeath()															{ /*assert(hasDied());*/ m_aidsDeath = true; }
	bool wasAIDSDeath() const														{ /*assert(hasDied());*/ return m_aidsDeath; }

	// This is a per person value
	double getSurvivalTimeLog10Offset() const										{ return m_log10SurvTimeOffset; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getViralLoadFromSetPointViralLoad(double x) const;
	void initializeCD4Counts();
	static double pickSeedSetPointViralLoad();
	static double pickInheritedSetPointViralLoad(const Person *pOrigin);

	const Person *m_pSelf;

	double m_infectionTime;
	Person *m_pInfectionOrigin;
	InfectionType m_infectionType;
	InfectionStage m_infectionStage;
	int m_diagnoseCount;
	bool m_aidsDeath;
	double m_log10SurvTimeOffset;

	double m_Vsp, m_VspOriginal;
	bool m_VspLowered;
	double m_lastTreatmentStartTime;
	int m_treatmentCount;

	AIDSTimeOfDeathUtility m_aidsTodUtil;

	double m_cd4AtStart, m_cd4AtDeath;
	double m_artAcceptanceThreshold;

	static double m_hivSeedWeibullShape;
	static double m_hivSeedWeibullScale;
	static double m_VspHeritabilitySigmaFraction;
	static double m_acuteFromSetPointParamX;
	static double m_aidsFromSetPointParamX;
	static double m_finalAidsFromSetPointParamX;
	static double m_maxValue;
	static double m_maxViralLoad;

	static VspModel *m_pVspModel;

	static ProbabilityDistribution *m_pCD4StartDistribution;
	static ProbabilityDistribution *m_pCD4EndDistribution;
	static ProbabilityDistribution *m_pARTAcceptDistribution;
	static ProbabilityDistribution *m_pLogSurvTimeOffsetDistribution;
};

inline double Person_HIV::getViralLoad() const
{ 
	assert(m_infectionStage != NoInfection); 
	if (m_infectionStage == Acute) 
		return getViralLoadFromSetPointViralLoad(m_acuteFromSetPointParamX); 
	else if (m_infectionStage == Chronic)
		return getSetPointViralLoad(); 
	else if (m_infectionStage == AIDS)
		return getViralLoadFromSetPointViralLoad(m_aidsFromSetPointParamX);
	else if (m_infectionStage == AIDSFinal)
		return getViralLoadFromSetPointViralLoad(m_finalAidsFromSetPointParamX);
	
	abortWithMessage("Unknown stage in Person::getViralLoad");
	return -1;
}

#endif // PERSON_HIV_H