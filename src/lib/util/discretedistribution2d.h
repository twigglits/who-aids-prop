#ifndef DISCRETEDISTRIBUTION2D_H

#define DISCRETEDISTRIBUTION2D_H

#include "probabilitydistribution2d.h"
#include "discretedistribution.h"
#include "discretedistributionfast.h"
#include "polygon2d.h"
#include <vector>

class TIFFDensityFile;

//#define OLDTEST

class DiscreteDistribution2D : public ProbabilityDistribution2D
{
public:
	DiscreteDistribution2D(double xOffset, double yOffset, double xSize, double ySize,
			       const TIFFDensityFile &density, GslRandomNumberGenerator *pRngGen,
			       const Polygon2D &filter = Polygon2D()
			       );
	~DiscreteDistribution2D();

	Point2D pickPoint() const;

	double getXOffset() const								{ return m_xOffset; }
	double getYOffset() const								{ return m_yOffset; }
	double getXSize() const									{ return m_xSize; }
	double getYSize() const									{ return m_ySize; }
private:
#ifdef OLDTEST
	DiscreteDistribution *m_pMarginalYDist;
	std::vector<DiscreteDistribution *> m_conditionalXDists;
#else
	DiscreteDistributionFast *m_pMarginalYDist;
	std::vector<DiscreteDistributionFast *> m_conditionalXDists;
#endif
	double m_xOffset, m_yOffset;
	double m_xSize, m_ySize;
	int m_width, m_height; // discrete size (pixels)
};

#endif // DISCRETEDISTRIBUTION2D_H
