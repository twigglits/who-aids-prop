#include "uniformdistribution.h"
#include "betadistribution.h"
#include "gammadistribution.h"
#include "fixedvaluedistribution.h"
#include "lognormaldistribution.h"
#include "normaldistribution.h"
#include "exponentialdistribution.h"
#include "fixedvaluedistribution2d.h"
#include "uniformdistribution2d.h"
#include "binormaldistribution.h"
#include "discretedistribution2d.h"
#include "tiffdensityfile.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include <vector>

using namespace std;

ProbabilityDistribution *getDistributionFromConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen, 
		                                   const string &prefix)
{
	vector<string> supportedDistributions;
	string distName;

	supportedDistributions.push_back("fixed");
	supportedDistributions.push_back("uniform");
	supportedDistributions.push_back("beta");
	supportedDistributions.push_back("gamma");
	supportedDistributions.push_back("lognormal");
	supportedDistributions.push_back("normal");
	supportedDistributions.push_back("exponential");
	
	if (!config.getKeyValue(prefix + ".dist.type", distName, supportedDistributions))
		abortWithMessage(config.getErrorString());

	ProbabilityDistribution *pDist = 0;
	if (distName == "fixed")
	{
		double value;

		if (!config.getKeyValue(prefix + ".dist.fixed.value", value))
			abortWithMessage(config.getErrorString());

		pDist = new FixedValueDistribution(value, pRndGen);
	}
	else if (distName == "uniform")
	{
		double minValue, maxValue;

		if (!config.getKeyValue(prefix + ".dist.uniform.min", minValue) ||
		    !config.getKeyValue(prefix + ".dist.uniform.max", maxValue, minValue) )
			abortWithMessage(config.getErrorString());

		pDist = new UniformDistribution(minValue, maxValue, pRndGen);
	}
	else if (distName == "beta")
	{
		double a, b, minVal, maxVal;

		if (!config.getKeyValue(prefix + ".dist.beta.a", a) ||
		    !config.getKeyValue(prefix + ".dist.beta.b", b) ||
		    !config.getKeyValue(prefix + ".dist.beta.min", minVal) ||
		    !config.getKeyValue(prefix + ".dist.beta.max", maxVal, minVal) )
			abortWithMessage(config.getErrorString());

		pDist = new BetaDistribution(a, b, minVal, maxVal, pRndGen);
	}
	else if (distName == "gamma")
	{
		double a, b;

		if (!config.getKeyValue(prefix + ".dist.gamma.a", a) ||
		    !config.getKeyValue(prefix + ".dist.gamma.b", b) )
			abortWithMessage(config.getErrorString());

		pDist = new GammaDistribution(a, b, pRndGen);
	}
	else if (distName == "lognormal")
	{
		double zeta, sigma;

		if (!config.getKeyValue(prefix + ".dist.lognormal.zeta", zeta) ||
		    !config.getKeyValue(prefix + ".dist.lognormal.sigma", sigma, 0) )
			abortWithMessage(config.getErrorString());

		pDist = new LogNormalDistribution(zeta, sigma, pRndGen);
	}
	else if (distName == "normal")
	{
		double mu, sigma, minValue, maxValue;

		if (!config.getKeyValue(prefix + ".dist.normal.mu", mu) ||
		    !config.getKeyValue(prefix + ".dist.normal.sigma", sigma, 0) ||
		    !config.getKeyValue(prefix + ".dist.normal.min", minValue) ||
		    !config.getKeyValue(prefix + ".dist.normal.max", maxValue, minValue)
		    )
			abortWithMessage(config.getErrorString());

		pDist = new NormalDistribution(mu, sigma, pRndGen, minValue, maxValue);
	}
	else if (distName == "exponential")
	{
		double lambda;

		if (!config.getKeyValue(prefix + ".dist.exponential.lambda", lambda, 0))
			abortWithMessage(config.getErrorString());

		pDist = new ExponentialDistribution(lambda, pRndGen);
	}
	else
		abortWithMessage("ERROR: unknown distribution name for " + prefix + ".dist.type:" + distName);

	assert(pDist != 0);
	return pDist;
}

void addDistributionToConfig(ProbabilityDistribution *pSrcDist, ConfigWriter &config, const std::string &prefix)
{
	if (pSrcDist == 0)
	{
		if (!config.addKey(prefix + ".dist.type", "NULL"))
			abortWithMessage(config.getErrorString());
		return;
	}

	// Just some curly braces to limite the name scope
	{
		FixedValueDistribution *pDist = 0;
		if ((pDist = dynamic_cast<FixedValueDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "fixed") ||
			    !config.addKey(prefix + ".dist.fixed.value", pDist->getValue()))
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limite the name scope
	{
		UniformDistribution *pDist = 0;
		if ((pDist = dynamic_cast<UniformDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "uniform") ||
			    !config.addKey(prefix + ".dist.uniform.min", pDist->getMin()) ||
			    !config.addKey(prefix + ".dist.uniform.max", pDist->getMin() + pDist->getRange()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}
	
	// Just some curly braces to limite the name scope
	{
		BetaDistribution *pDist = 0;
		if ((pDist = dynamic_cast<BetaDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "beta") ||
			    !config.addKey(prefix + ".dist.beta.a", pDist->getA()) ||
			    !config.addKey(prefix + ".dist.beta.b", pDist->getB()) ||
			    !config.addKey(prefix + ".dist.beta.min", pDist->getMin()) ||
			    !config.addKey(prefix + ".dist.beta.max", pDist->getMin() + pDist->getScale()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limite the name scope
	{
		GammaDistribution *pDist = 0;
		if ((pDist = dynamic_cast<GammaDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "gamma") ||
			    !config.addKey(prefix + ".dist.gamma.a", pDist->getA()) ||
			    !config.addKey(prefix + ".dist.gamma.b", pDist->getB()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limite the name scope
	{
		LogNormalDistribution *pDist = 0;
		if ((pDist = dynamic_cast<LogNormalDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "lognormal") ||
			    !config.addKey(prefix + ".dist.lognormal.zeta", pDist->getZeta()) ||
			    !config.addKey(prefix + ".dist.lognormal.sigma", pDist->getSigma()))
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limite the name scope
	{
		NormalDistribution *pDist = 0;
		if ((pDist = dynamic_cast<NormalDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "normal") ||
			    !config.addKey(prefix + ".dist.normal.mu", pDist->getMu()) ||
			    !config.addKey(prefix + ".dist.normal.sigma", pDist->getSigma()) ||
			    !config.addKey(prefix + ".dist.normal.min", pDist->getMin()) ||
			    !config.addKey(prefix + ".dist.normal.max", pDist->getMax())
			    )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limite the name scope
	{
		ExponentialDistribution *pDist = 0;
		if ((pDist = dynamic_cast<ExponentialDistribution *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist.type", "exponential") ||
			    !config.addKey(prefix + ".dist.exponential.lambda", pDist->getA()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	abortWithMessage("addDistributionToConfig: specified unknown distribution!");
}

ProbabilityDistribution2D *getDistribution2DFromConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen,
		                                       const std::string &prefix)
{
	vector<string> supportedDistributions;
	string distName;

	supportedDistributions.push_back("fixed");
	supportedDistributions.push_back("uniform");
	supportedDistributions.push_back("binormal");
	supportedDistributions.push_back("binormalsymm");
	supportedDistributions.push_back("discrete");
	
	if (!config.getKeyValue(prefix + ".dist2d.type", distName, supportedDistributions))
		abortWithMessage(config.getErrorString());

	ProbabilityDistribution2D *pDist = 0;
	if (distName == "fixed")
	{
		double xvalue, yvalue;

		if (!config.getKeyValue(prefix + ".dist2d.fixed.xvalue", xvalue) ||
		    !config.getKeyValue(prefix + ".dist2d.fixed.yvalue", yvalue) )
			abortWithMessage(config.getErrorString());

		pDist = new FixedValueDistribution2D(xvalue, yvalue, pRndGen);
	}
	else if (distName == "uniform")
	{
		double minXValue, maxXValue;
		double minYValue, maxYValue;

		if (!config.getKeyValue(prefix + ".dist2d.uniform.xmin", minXValue) ||
		    !config.getKeyValue(prefix + ".dist2d.uniform.xmax", maxXValue, minXValue) ||
		    !config.getKeyValue(prefix + ".dist2d.uniform.ymin", minYValue) ||
		    !config.getKeyValue(prefix + ".dist2d.uniform.ymax", maxYValue, minYValue) )
			abortWithMessage(config.getErrorString());

		pDist = new UniformDistribution2D(minXValue, maxXValue, minYValue, maxYValue, pRndGen);
	}
	else if (distName == "binormal")
	{
		double xMean, xSigma, xMin, xMax;
		double yMean, ySigma, yMin, yMax;
		double rho;

		if (!config.getKeyValue(prefix + ".dist2d.binormal.meanx", xMean) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.meany", yMean) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.sigmax", xSigma) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.sigmay", ySigma) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.rho", rho, -1, 1) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.minx", xMin) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.maxx", xMax, xMin) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.miny", yMin) ||
		    !config.getKeyValue(prefix + ".dist2d.binormal.maxy", yMax, yMin)
		    )
			abortWithMessage(config.getErrorString());

		pDist = new BinormalDistribution(xMean, yMean, xSigma, ySigma, rho, pRndGen, xMin, xMax, yMin, yMax);
	}
	else if (distName == "binormalsymm")
	{
		double xMean, xSigma, xMin, xMax;
		double rho;

		if (!config.getKeyValue(prefix + ".dist2d.binormalsymm.mean", xMean) ||
		    !config.getKeyValue(prefix + ".dist2d.binormalsymm.sigma", xSigma) ||
		    !config.getKeyValue(prefix + ".dist2d.binormalsymm.rho", rho, -1, 1) ||
		    !config.getKeyValue(prefix + ".dist2d.binormalsymm.min", xMin) ||
		    !config.getKeyValue(prefix + ".dist2d.binormalsymm.max", xMax, xMin) 
		    )
			abortWithMessage(config.getErrorString());

		pDist = new BinormalDistribution(xMean, xSigma, rho, pRndGen, xMin, xMax);
	}
	else if (distName == "discrete")
	{
		string tiffFileName, maskFileName;
		double xOffset, yOffset, width, height;
		bool flipy;

		if (!config.getKeyValue(prefix + ".dist2d.discrete.densfile", tiffFileName) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.maskfile", maskFileName) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.xoffset", xOffset) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.yoffset", yOffset) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.width", width) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.height", height) ||
		    !config.getKeyValue(prefix + ".dist2d.discrete.flipy", flipy)
		   )
			abortWithMessage(config.getErrorString());

		TIFFDensityFile tiffFile;

		if (!tiffFile.init(tiffFileName, true, flipy))
			abortWithMessage("Unable to read specified TIFF density file: " + tiffFile.getErrorString());

		if (maskFileName.length() > 0) // A mask file was specified
		{
			TIFFDensityFile maskFile;

			if (!maskFile.init(maskFileName, false, flipy))
				abortWithMessage("Unable to read specified TIFF mask file: " + maskFile.getErrorString());

			int w = tiffFile.getWidth();
			int h = tiffFile.getHeight();

			if (!(maskFile.getWidth() == w && maskFile.getHeight() == h))
				abortWithMessage("Dimensions of density file '" + tiffFileName + "' and mask file '" + maskFileName + "' don't match");

			for (int y = 0 ; y < h ; y++)
			{
				for (int x = 0 ; x < w ; x++)
				{
					double maskVal = maskFile.getValue(x, y);
					if (maskVal <= 0)
						tiffFile.setValue(x, y, 0);
				}
			}
		}

		pDist = new DiscreteDistribution2D(xOffset, yOffset, width, height, tiffFile, pRndGen);
	}
	else
		abortWithMessage("ERROR: unknown 2D distribution name for " + prefix + ".dist2d.type:" + distName);

	assert(pDist != 0);
	return pDist;
}

void addDistribution2DToConfig(ProbabilityDistribution2D *pSrcDist, ConfigWriter &config, const std::string &prefix)
{
	if (pSrcDist == 0)
	{
		if (!config.addKey(prefix + ".dist2d.type", "NULL"))
			abortWithMessage(config.getErrorString());
		return;
	}

	// Just some curly braces to limit the name scope
	{
		FixedValueDistribution2D *pDist = 0;
		if ((pDist = dynamic_cast<FixedValueDistribution2D *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist2d.type", "fixed") ||
			    !config.addKey(prefix + ".dist2d.fixed.xvalue", pDist->getXValue()) ||
			    !config.addKey(prefix + ".dist2d.fixed.yvalue", pDist->getYValue()) 
			   )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limit the name scope
	{
		UniformDistribution2D *pDist = 0;
		if ((pDist = dynamic_cast<UniformDistribution2D *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist2d.type", "uniform") ||
			    !config.addKey(prefix + ".dist2d.uniform.xmin", pDist->getXMin()) ||
			    !config.addKey(prefix + ".dist2d.uniform.xmax", pDist->getXMax()) ||
			    !config.addKey(prefix + ".dist2d.uniform.ymin", pDist->getYMin()) ||
			    !config.addKey(prefix + ".dist2d.uniform.ymax", pDist->getYMax()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	// Just some curly braces to limit the name scope
	{
		BinormalDistribution *pDist = 0;
		if ((pDist = dynamic_cast<BinormalDistribution *>(pSrcDist)) != 0)
		{
			if (pDist->isSymmetric())
			{
				if (!config.addKey(prefix + ".dist2d.type", "binormalsymm") ||
				    !config.addKey(prefix + ".dist2d.binormalsymm.mean", pDist->getMeanX()) ||
				    !config.addKey(prefix + ".dist2d.binormalsymm.sigma", pDist->getSigmaX()) ||
				    !config.addKey(prefix + ".dist2d.binormalsymm.rho", pDist->getRho()) ||
				    !config.addKey(prefix + ".dist2d.binormalsymm.min", pDist->getMinX()) ||
				    !config.addKey(prefix + ".dist2d.binormalsymm.max", pDist->getMaxX()) )
					abortWithMessage(config.getErrorString());

				return;
			}
			else
			{
				if (!config.addKey(prefix + ".dist2d.type", "binormal") ||
				    !config.addKey(prefix + ".dist2d.binormal.rho", pDist->getRho()) ||
				    !config.addKey(prefix + ".dist2d.binormal.meanx", pDist->getMeanX()) ||
				    !config.addKey(prefix + ".dist2d.binormal.sigmax", pDist->getSigmaX()) ||
				    !config.addKey(prefix + ".dist2d.binormal.minx", pDist->getMinX()) ||
				    !config.addKey(prefix + ".dist2d.binormal.maxx", pDist->getMaxX()) ||
				    !config.addKey(prefix + ".dist2d.binormal.meany", pDist->getMeanY()) ||
				    !config.addKey(prefix + ".dist2d.binormal.sigmay", pDist->getSigmaY()) ||
				    !config.addKey(prefix + ".dist2d.binormal.miny", pDist->getMinY()) ||
				    !config.addKey(prefix + ".dist2d.binormal.maxy", pDist->getMaxY()) )
					abortWithMessage(config.getErrorString());

				return;
			}
		}
	}

	// Just some curly braces to limit the name scope
	{
		DiscreteDistribution2D *pDist = 0;
		if ((pDist = dynamic_cast<DiscreteDistribution2D *>(pSrcDist)) != 0)
		{
			if (!config.addKey(prefix + ".dist2d.type", "discrete") ||
			    !config.addKey(prefix + ".dist2d.discrete.densfile", "IGNORE") ||
			    !config.addKey(prefix + ".dist2d.discrete.maskfile", "IGNORE") ||
			    !config.addKey(prefix + ".dist2d.discrete.xoffset", pDist->getXOffset()) ||
			    !config.addKey(prefix + ".dist2d.discrete.yoffset", pDist->getYOffset()) ||
			    !config.addKey(prefix + ".dist2d.discrete.width", pDist->getXSize()) ||
			    !config.addKey(prefix + ".dist2d.discrete.height", pDist->getYSize()) ||
			    !config.addKey(prefix + ".dist2d.discrete.flipy", pDist->isYFlipped()) )
				abortWithMessage(config.getErrorString());

			return;
		}
	}

	abortWithMessage("addDistribution2DToConfig: specified unknown distribution!");
}

JSONConfig distributionJSONConfig("distTypes", R"JSON(
        "fixed": { 
            "params": [ ["value", 0] ],
            "info": [ 
                "Each time a value is picked from this 'distribution', the specified fixed ",
                "value is returned." 
            ]
        },
        "uniform": {
            "params": [ ["min", 0], ["max", 1] ],
            "info": [ "Parameters for a uniform distribution" ]
        },
        "beta": { 
            "params": [ ["a", null], ["b", null], ["min", null], ["max", null] ],
            "info": [ 
                "Parameters for a beta distribution (rescaled)",
                "prob(x) = gamma(a+b)/(gamma(a)*gamma(b))*((x-min)/(max-min))^(a-1.0)*(1.0-((x-min)/(max-min)))^(b-1.0) * 1.0/(max-min)" 
            ]
        },
        "gamma": { 
            "params": [ ["a", null], ["b", null] ],
            "info": [ 
                "Parameters for a gamma distribution",
                "prob(x) = (x^(a-1.0))*exp(-x/b)/((b^a)*gamma(a))"
            ]
        },
        "lognormal": { 
            "params": [ ["zeta", null], ["sigma", null] ],
            "info": [ 
                "Parameters for a log-normal distribution",
                "prob(x) = 1.0/(x*s*sqrt(2.0*pi)) * exp(- (ln(x)-z)^2 / (2.0*s^2))"
            ]
        },
        "normal": { 
            "params": [ ["mu", null], ["sigma", null], ["min", "-inf"], ["max", "inf"] ],
            "info": [ 
                "Parameters for a clipped normal distribution",
                "prob(x) = 1.0/(s*sqrt(2.0*pi)) * exp(- (x-m)^2 / (2.0*s^2))",
        "possibly truncated to [min,max] (using rejection sampling)"
            ]
        },
        "exponential": {
            "params": [ [ "lambda", null ] ],
            "info": [
                "Parameters for an exponential distribution",
                "prob(x) = lambda * exp(-lambda * x)"
            ]
        })JSON");

JSONConfig distribution2DJSONConfig("distTypes2D", R"JSON(
        "fixed": {
            "params": [ 
                [ "xvalue", 0 ], 
                [ "yvalue", 0 ]
             ],
            "info": [ 
                "Each time a value from this 'distribution' is picked, the specified x- and",
                "y- values will be returned."
            ]
        },
        "uniform": {
            "params": [
                [ "xmin", 0 ],
                [ "xmax", 1 ],
                [ "ymin", 0 ],
                [ "ymax", 1 ]
            ],
            "info": [ "Parameters for a 2D uniform distribution." ]
        },
        "binormal": {
            "params": [ 
                [ "meanx", 0 ], 
                [ "meany", 0 ],
                [ "sigmax", 1 ],
                [ "sigmay", 1 ],
                [ "rho", 0 ],
                [ "minx", "-inf" ],
                [ "maxx", "inf" ],
                [ "miny", "-inf" ],
                [ "maxy", "inf" ]
            ],
            "info": [ 
                "Parameters for a clipped binormal distribution, based on",
                "prob(x,y) = 1.0/(2.0*pi*sigmax*sigmay*sqrt(1.0-rho^2))",
                "          * exp[-(x^2/sigmax^2 + y^2/sigmay^2",
                "                  - 2.0*rho*x*y/(sigmax*sigmay))/(2.0*(1.0-rho^2))]",
                "Clipping to the specified region is done using rejection sampling"
            ]
        },
        "binormalsymm": {
            "params": [ 
                [ "mean", 0 ], 
                [ "sigma", 1 ],
                [ "rho", 0 ],
                [ "min", "-inf" ],
                [ "max", "inf" ]
            ],
            "info": [ 
                "Parameters for a symmetric clipped binormal distribution, based on",
                "prob(x,y) = 1.0/(2.0*pi*sigma^2*sqrt(1.0-rho^2))",
                "          * exp[-(x^2 + y^2 - 2.0*rho*x*y)/(2.0*sigma^2*(1.0-rho^2))]",
                "Clipping to the specified region is done using rejection sampling"
            ]
        },
        "discrete": {
            "params": [
                [ "densfile", null ],
                [ "maskfile", null ],
                [ "xoffset", 0 ],
                [ "yoffset", 0 ],
                [ "width", 1 ],
                [ "height", 1 ],
                [ "flipy", "yes", [ "yes", "no"] ]
            ],
            "info": [ 
                "The 'densfile' parameter specifies a TIFF file you want to use to base a",
                "discrete probability distribution on. If specified, a mask file can be used",
                "to limit the distribution to a portion of the file: this mask file should",
                "have the same number of pixels as the density file, with pixels having a",
                "value of zero or negative meaning that that part of the density file is",
                "ignored.",
                "",
                "The offsets and width/height parameters specify how the pixel coordinates",
                "should be translated into an (x,y) pair generated from the distribution.",
                "",
                "Because of the way pixels are stored in a TIFF file, the y-axis would",
                "point down and the (0,0) pixel would be the upper-left pixel. This is what",
                "is used when 'flipy' is set to 'no'. The default however is 'yes', which",
                "causes the lower-left pixel to be the (0,0) pixel of the TIFF file, and",
                "which causes the y-axis to point up, as usually will be desired."
            ]
        })JSON");

