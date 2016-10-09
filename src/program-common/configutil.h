#ifndef CONFIGUTIL_H

#define CONFIGUTIL_H

#include <stdint.h>

class ConfigSettings;
class GslRandomNumberGenerator;
class SimpactPopulationConfig;
class PopulationDistributionCSV;

void showConfigOptions();
void processNonInterventionEventConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
bool configure(ConfigSettings &config, SimpactPopulationConfig &populationConfig, PopulationDistributionCSV &ageDist,
	       GslRandomNumberGenerator *pRndGen, double &tMax, int64_t &maxEvents);
void checkConfiguration(const ConfigSettings &loadedConfig, const SimpactPopulationConfig &populationConfig, double tMax,
		        int64_t maxEvents);

#endif // CONFIGUTIL_H
