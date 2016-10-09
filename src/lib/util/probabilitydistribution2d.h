#ifndef PROBABILITYDISTRIBUTION2D_H

#define PROBABILITYDISTRIBUTION2D_H

/**
 * \file probabilitydistribution2d.h
 */

#include <assert.h>

class GslRandomNumberGenerator;

struct Point2D
{
	Point2D() 										{ }
	Point2D(double x0, double y0) 								{ x = x0; y = y0; }

	double x, y;
};

/** Abstract base class for 2D probability distribution implementations so that they can
 *  be used interchangeably. */
class ProbabilityDistribution2D
{
public:
	ProbabilityDistribution2D(GslRandomNumberGenerator *pRng)			{ assert(pRng != 0); m_pRng = pRng; }
	virtual ~ProbabilityDistribution2D()						{ }

	/** Pick a point according to a specific distrubution, specified in a subclass 
	 *  of ProbabilityDistribution2D . */
	virtual Point2D pickPoint() const = 0;
	GslRandomNumberGenerator *getRandomNumberGenerator() const			{ return m_pRng; }
private:
	mutable GslRandomNumberGenerator *m_pRng;
};

#endif // PROBABILITYDISTRIBUTION2D_H
