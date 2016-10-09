#ifndef DISCRETEDISTRIBUTION2D_H

#define DISCRETEDISTRIBUTION2D_H

#include "probabilitydistribution2d.h"
#include "discretedistributionfast.h"
#include <vector>

class TIFFDensityFile;

class DiscreteDistribution2D : public ProbabilityDistribution2D
{
public:
	DiscreteDistribution2D(double xOffset, double yOffset, double xSize, double ySize,
			       const TIFFDensityFile &density, GslRandomNumberGenerator *pRngGen);
	~DiscreteDistribution2D();

	Point2D pickPoint() const;
private:
	DiscreteDistributionFast *m_pMarginalYDist;
	std::vector<DiscreteDistributionFast *> m_conditionalXDists;
};

#endif // DISCRETEDISTRIBUTION2D_H
