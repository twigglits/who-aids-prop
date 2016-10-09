#include "discretedistribution2d.h"
#include "tiffdensityfile.h"
#include <iostream>

using namespace std;

DiscreteDistribution2D::DiscreteDistribution2D(double xOffset, double yOffset, double xSize, double ySize,
			       		       const TIFFDensityFile &density, GslRandomNumberGenerator *pRngGen) : ProbabilityDistribution2D(pRngGen)
{
	m_pMarginalYDist = 0;

	int width = density.getWidth();
	int height = density.getHeight();

	vector<double> yValues(height);
	vector<double> xValues(width);

	for (size_t y = 0 ; y < height ; y++)
	{
		double sum = 0;

		for (size_t x = 0 ; x < width ; x++)
		{
			float val = density.getValue(x, y);

			assert(val >= 0);

			xValues[x] = val;
			sum += val;
		}

		m_conditionalXDists.push_back(new DiscreteDistributionFast(xOffset, xOffset+xSize, xValues, pRngGen));

		yValues[y] = sum;
	}

	m_pMarginalYDist = new DiscreteDistributionFast(yOffset, yOffset+ySize, yValues, pRngGen);
}

DiscreteDistribution2D::~DiscreteDistribution2D()
{
	delete m_pMarginalYDist;
	for (size_t i = 0 ; i < m_conditionalXDists.size() ; i++)
		delete m_conditionalXDists[i];
}

Point2D DiscreteDistribution2D::pickPoint() const
{
	double y = m_pMarginalYDist->pickNumber();
	int yi = (int)y;
	assert(yi >= 0 && yi < m_conditionalXDists.size());
	
	double x = m_conditionalXDists[yi]->pickNumber();

	return Point2D(x, y);
}
