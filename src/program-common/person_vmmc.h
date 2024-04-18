#ifndef PERSON_VMMC_H

#define PERSON_VMMC_H

#include "util.h"

class Person;
class ProbabilityDistribution;
class VspModel;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_VMMC
{
public:
	Person_VMMC(Person *pSelf);
	~Person_VMMC();

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:

	const Person *m_pSelf;
	Person *m_pInfectionOrigin;
	double m_artAcceptanceThreshold;
	static VspModel *m_pVspModel;

	static ProbabilityDistribution *m_pCD4StartDistribution;
	static ProbabilityDistribution *m_pCD4EndDistribution;
	static ProbabilityDistribution *m_pARTAcceptDistribution;
	static ProbabilityDistribution *m_pLogSurvTimeOffsetDistribution;
	static ProbabilityDistribution *m_pB0Dist;
	static ProbabilityDistribution *m_pB1Dist;
};

#endif // PERSON_VMMC_H
