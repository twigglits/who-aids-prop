#ifndef EVTHAZARD_H

#define EVTHAZARD_H

class SimpactPopulation;
class SimpactEvent;
class ConfigWriter;

// WARNING: the same instance can be called from multiple threads
class EvtHazard
{
public:
	EvtHazard()									{ }
	virtual ~EvtHazard()								{ }

	virtual double calculateInternalTimeInterval(const SimpactPopulation &population, 
	                                             const SimpactEvent &evt, double t0, double dt) = 0;
	virtual double solveForRealTimeInterval(const SimpactPopulation &population,
			                        const SimpactEvent &evt, double Tdiff, double t0) = 0;

	virtual void obtainConfig(ConfigWriter &config) = 0;
};

#endif // EVTHAZARD_H

