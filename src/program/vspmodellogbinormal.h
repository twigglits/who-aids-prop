#ifndef VSPMODELLOGBINORMAL_H

#define VSPMODELLOGBINORMAL_H

#include "vspmodel.h"
#include <utility>

class GslRandomNumberGenerator;
class ProbabilityDistribution;

class VspModelLogBiNormal : public VspModel
{
public:
	// Takes control over pAltSeedDist 
	VspModelLogBiNormal(double mu, double sigma, double rho, double minVal, double maxVal, ProbabilityDistribution *pAltSeedDist, GslRandomNumberGenerator *pRndGen);
	~VspModelLogBiNormal();

	double pickSetPointViralLoad();
	double inheritSetPointViralLoad(double VspInfector);

	std::pair<double,double> pickBiNormal();

	double getMean() const										{ return m_mu; }
	double getSigma() const										{ return m_sigma; }
	double getRho() const										{ return m_rho; }
	double getMin() const										{ return m_min; }
	double getMax() const										{ return m_max; }
	ProbabilityDistribution *getAltSeedDist() const							{ return m_pAltSeedDist; }
private:
	double m_mu, m_sigma, m_rho;
	double m_min, m_max;
	ProbabilityDistribution *m_pAltSeedDist;
};

#endif // VSPMODELLOGBINORMAL_H

