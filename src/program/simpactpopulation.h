#ifndef SIMPACTPOPULATION_H

#define SIMPACTPOPULATION_H

#include "population.h"
#include <assert.h>

class PopulationDistribution;
class Person;
class Man;
class Woman;

class SimpactPopulationConfig : public errut::ErrorBase
{
public:
	SimpactPopulationConfig();
	~SimpactPopulationConfig();

	void setInitialMen(int number)						{ m_initialMen = number; }
	void setInitialWomen(int number)					{ m_initialWomen = number; }
	void setInitialInfectionFraction(double f)				{ m_initialInfectionFraction = f; }

	int getInitialMen() const						{ return m_initialMen; }
	int getInitialWomen() const						{ return m_initialWomen; }
	double getInitialInfectionFraction() const				{ return m_initialInfectionFraction; }
private:
	int m_initialMen, m_initialWomen;
	double m_initialInfectionFraction;
};

class SimpactPopulation : public Population
{
public:
	SimpactPopulation(bool parallel, GslRandomNumberGenerator *pRng);
	~SimpactPopulation();

	bool init(const SimpactPopulationConfig &popConfig, const PopulationDistribution &popDist);

	Person **getAllPeople()							{ return reinterpret_cast<Person**>(Population::getAllPeople()); }
	Man **getMen()								{ return reinterpret_cast<Man**>(Population::getMen()); }
	Woman **getWomen()							{ return reinterpret_cast<Woman**>(Population::getWomen()); }

	int getInitialPopulationSize() const					{ return m_initialPopulationSize; }
	double getDebutAge() const						{ return 15.0; }
	double getAcuteStageTime() const					{ return 3.0/12.0; } // three months 
protected:
	virtual void onScheduleInitialEvents();
private:
	void onAboutToFire(EventBase *pEvt);

	int m_initialPopulationSize;

	bool m_init;
};

inline SimpactPopulation &SIMPACTPOPULATION(State *pState)
{
	assert(pState != 0);
	return static_cast<SimpactPopulation &>(*pState);
}

inline const SimpactPopulation &SIMPACTPOPULATION(const State *pState)
{
	assert(pState != 0);
	return static_cast<const SimpactPopulation &>(*pState);
}

#endif // SIMPACTPOPULATION_H

