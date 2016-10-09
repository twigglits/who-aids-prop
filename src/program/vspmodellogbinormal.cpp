#include "vspmodellogbinormal.h"
#include "gslrandomnumbergenerator.h"
#include "probabilitydistribution.h"
#include "util.h"
#include <assert.h>
#include <iostream>
#include <cmath>

using namespace std;

VspModelLogBiNormal::VspModelLogBiNormal(double mu, double sigma, double rho, double minVal, double maxVal, 
		                         ProbabilityDistribution *pAltSeedDist, GslRandomNumberGenerator *pRndGen) : VspModel(pRndGen)
{
	if (!(minVal < maxVal))
		abortWithMessage("VspModelLogBiNormal: Truncation boundaries must be ordered correctly!");
	if (!(rho <= 1.0 && rho >= -1.0))
		abortWithMessage("VspModelLogBiNormal: rho must lie between -1 and 1");

	m_mu = mu;
	m_sigma = sigma;
	m_rho = rho;
	m_min = minVal;
	m_max = maxVal;
	m_pAltSeedDist = pAltSeedDist;
}

VspModelLogBiNormal::~VspModelLogBiNormal()
{
}

double VspModelLogBiNormal::pickSetPointViralLoad()
{
	int count = 0;
	const int maxCount = 100000;
	double x;

	if (m_pAltSeedDist) // We want to use an alternative seeding distribution
	{
		x = m_pAltSeedDist->pickNumber();
		while ((x < m_min || x > m_max) && count++ < maxCount)
			x = m_pAltSeedDist->pickNumber();

	}
	else
	{
		/*
		// This is the marginal, so if we sample from the 2D dist and just keep one
		// it should be OK
		
		std::pair<double,double> xy = pickBiNormal();

		// TODO: it matters which one we keep if the distribution is not symmetric
		double x = xy.first;
		*/

		// Checked that this yields the same result as just sampling from 1D gaussian(mu,sigma),
		// (also with maple)
		x = getRandomNumberGenerator()->pickGaussianNumber(m_mu, m_sigma);
		while ((x < m_min || x > m_max) && count++ < maxCount) // rejection sampling
			x = getRandomNumberGenerator()->pickGaussianNumber(m_mu, m_sigma);
	}

	if (count >= maxCount)
		abortWithMessage("Couldn't find an appropriate seed SPVL value (always lies outside the min/max bounds)");

	return std::pow(10.0,x); // use this as power of 10
}

// Just for testing, not actually used
std::pair<double,double> VspModelLogBiNormal::pickBiNormal()
{
	GslRandomNumberGenerator *pRndGen = getRandomNumberGenerator();
	std::pair<double,double> xy = pRndGen->pickBivariateGaussian(m_mu, m_mu, m_sigma, m_sigma, m_rho);
	
	// Re-sample if not inside boundaries (rejection sampling)
	while (xy.first < m_min || xy.first > m_max || xy.second < m_min || xy.second > m_max)
		xy = pRndGen->pickBivariateGaussian(m_mu, m_mu, m_sigma, m_sigma, m_rho);

	return xy;
}

double VspModelLogBiNormal::inheritSetPointViralLoad(double VspInfector)
{
	double x = VspInfector;
	x = std::log10(x); // have to switch back to the log scale
	
	// If my calculations are correct, this should be the mean and sigma
	// of the resulting conditional prob(y|x)
	double sigma = m_sigma*std::sqrt(1.0-m_rho*m_rho);
	double mean = m_mu + m_rho*(x-m_mu);

	GslRandomNumberGenerator *pRndGen = getRandomNumberGenerator();
	double y = pRndGen->pickGaussianNumber(mean, sigma);

	// Re-sample if not inside boundaries (rejection sampling)
	while (y < m_min || y > m_max)
		y = pRndGen->pickGaussianNumber(mean, sigma);

	return std::pow(10.0,y);
}


