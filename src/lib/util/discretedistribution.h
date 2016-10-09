#ifndef DISCRETEDISTRIBUTION_H

#define DISCRETEDISTRIBUTION_H

/**
 * \file discretedistribution.h
 */

#include <vector>

class GslRandomNumberGenerator;

/** Helper class to generate random numbers based on some kind of
 *  discrete distribution.
 *  You'll need to specify the sizes of the bins and the values
 *  of the bins, which are a measure of the integrated probability
 *  density inside bin.
 */
class DiscreteDistribution
{
public:
	/** Constructor of the class.
	 *  \param binStarts The values of the start of each bin. These
	 *                   must be in ascending order.
	 *  \param histValues Measures of the integrated probability in
	 *                    each bin
	 *  \param pRndGen The random number generator to use for randomness when
	 *                 picking numbers according to this distribution.
	 *
	 *  The value at the start of the last bin should be zero.
	 */
	DiscreteDistribution(std::vector<double> &binStarts,
			     std::vector<double> &histValues, 
			     GslRandomNumberGenerator *pRndGen);
	~DiscreteDistribution();

	/** Pick a number according to the discrete distrubution specified in the constructor. */
	double pickNumber() const;
private:
	std::vector<double> m_histSums;
	std::vector<double> m_binStarts;
	double m_totalSum;
	mutable GslRandomNumberGenerator *m_pRndGen;
};

#endif // DISCRETEDISTRIBUTION_H

