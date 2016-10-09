#include "discretedistribution2d.h"
#include "tiffdensityfile.h"
#include <iostream>

using namespace std;

DiscreteDistribution2D::DiscreteDistribution2D(double xOffset, double yOffset, double xSize, double ySize,
			       		       const TIFFDensityFile &density, GslRandomNumberGenerator *pRngGen,
					       const Polygon2D &filter) : ProbabilityDistribution2D(pRngGen)
{
	m_pMarginalYDist = 0;
	m_xSize = xSize;
	m_ySize = ySize;
	m_xOffset = xOffset;
	m_yOffset = yOffset;

	m_width = density.getWidth();
	m_height = density.getHeight();

	vector<double> yValues(m_height);
	vector<double> xValues(m_width);

	bool hasFilter = (filter.getNumberOfPoints() > 0);
	double pixelWidth = xSize/(double)m_width;
	double pixelHeight = ySize/(double)m_height;
	bool hasValue = false;

	for (size_t y = 0 ; y < m_height ; y++)
	{
		double sum = 0;

		for (size_t x = 0 ; x < m_width ; x++)
		{
			float val = 0;
			bool pass = true;
			
			if (hasFilter)
			{
				double xCoord = xOffset + pixelWidth*(0.5+(double)x);
				double yCoord = yOffset + pixelHeight*(0.5+(double)y);

				if (!filter.isInside(xCoord, yCoord))
					pass = false;
			}

			if (pass)
			{
				val = density.getValue(x, y);
				if (val != 0)
					hasValue = true;
			}

			assert(val >= 0);

			xValues[x] = val;
			sum += val;
		}

#ifdef OLDTEST
		std::vector<double> binStarts, histValues;
		for (int i = 0 ; i < xValues.size() ; i++)
		{
			binStarts.push_back(xOffset + (double)i);
			histValues.push_back(xValues[i]);
		}
		binStarts.push_back(m_width);
		histValues.push_back(0);

		m_conditionalXDists.push_back(new DiscreteDistribution(binStarts, histValues, pRngGen));
#else
		m_conditionalXDists.push_back(new DiscreteDistributionFast(0, m_width, xValues, pRngGen));
#endif

		yValues[y] = sum;
	}

	assert(hasValue);

#ifdef OLDTEST
	std::vector<double> binStarts, histValues;
	for (int i = 0 ; i < xValues.size() ; i++)
	{
		binStarts.push_back(yOffset + (double)i);
		histValues.push_back(yValues[i]);
	}
	binStarts.push_back(m_height);
	histValues.push_back(0);

	m_pMarginalYDist = new DiscreteDistribution(binStarts, histValues, pRngGen);
#else
	m_pMarginalYDist = new DiscreteDistributionFast(0, m_height, yValues, pRngGen);
#endif
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

	x = (x/(double)m_width)*m_xSize + m_xOffset;
	y = (y/(double)m_height)*m_ySize + m_yOffset;

	return Point2D(x, y);
}

