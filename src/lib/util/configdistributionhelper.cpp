#include "uniformdistribution.h"
#include "betadistribution.h"
#include "gammadistribution.h"
#include "fixedvaluedistribution.h"
#include "lognormaldistribution.h"
#include "normaldistribution.h"
#include "exponentialdistribution.h"
#include "configsettings.h"
#include "configwriter.h"
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
		double mu, sigma;

		if (!config.getKeyValue(prefix + ".dist.normal.mu", mu) ||
		    !config.getKeyValue(prefix + ".dist.normal.sigma", sigma, 0) )
			abortWithMessage(config.getErrorString());

		pDist = new NormalDistribution(mu, sigma, pRndGen);
	}
	else if (distName == "exponential")
	{
		double lambda;

		if (!config.getKeyValue(prefix + ".dist.exponential.lambda", lambda, 0))
			abortWithMessage(config.getErrorString());

		pDist = new ExponentialDistribution(lambda, pRndGen);
	}
	else
		abortWithMessage("ERROR: distribution name for " + prefix + ".dist.type:" + distName);

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
			    !config.addKey(prefix + ".dist.normal.sigma", pDist->getSigma()))
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

