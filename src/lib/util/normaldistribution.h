#ifndef NORMALDISTRIBUTION_H

#define NORMALDISTRIBUTION_H

/**
 * \file normaldistribution.h
 */

#include "probabilitydistribution.h"
#include "gslrandomnumbergenerator.h"

/** This class allows you to return a random number from a normal distribution
 *  with parameters specified in the constructor.
 *
 *  The probability density is the following: \f[ \textrm{prob}(x) = \frac{1}{\sigma \sqrt{2 \pi} }  \exp\left(-\frac{(x-\mu)^2}{2 \sigma^2}\right) \f]
 */
class NormalDistribution : public ProbabilityDistribution
{
public:
	/** The constructor specifies parameters for a log-normal distribution. */
	NormalDistribution(double mu, double sigma, GslRandomNumberGenerator *pRng);
	
	double pickNumber() const;
	double getMu() const									{ return m_mu; }
	double getSigma() const									{ return m_sigma; }
private:
	double m_mu, m_sigma;
};

inline NormalDistribution::NormalDistribution(double mu, double sigma, GslRandomNumberGenerator *pRng) : ProbabilityDistribution(pRng)
{
	m_mu = mu;
	m_sigma = sigma;
}

inline double NormalDistribution::pickNumber() const
{
	return getRandomNumberGenerator()->pickGaussianNumber(m_mu, m_sigma);
}

#endif // NORMALDISTRIBUTION_H

