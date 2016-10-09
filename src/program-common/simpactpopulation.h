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

	void setInitialMen(int number)									{ m_initialMen = number; }
	void setInitialWomen(int number)								{ m_initialWomen = number; }
	void setEyeCapsFraction(double f)								{ m_eyeCapsFraction = f; }

	int getInitialMen() const										{ return m_initialMen; }
	int getInitialWomen() const										{ return m_initialWomen; }
	double getEyeCapsFraction() const								{ return m_eyeCapsFraction; }
private:
	int m_initialMen, m_initialWomen;
	double m_eyeCapsFraction;
};

class SimpactPopulation : public Population
{
public:
	SimpactPopulation(bool parallel, GslRandomNumberGenerator *pRng);
	~SimpactPopulation();

	bool init(const SimpactPopulationConfig &popConfig, const PopulationDistribution &popDist);

	Person **getAllPeople()											{ return reinterpret_cast<Person**>(Population::getAllPeople()); }
	Man **getMen()													{ return reinterpret_cast<Man**>(Population::getMen()); }
	Woman **getWomen()												{ return reinterpret_cast<Woman**>(Population::getWomen()); }
	Person **getDeceasedPeople()									{ return reinterpret_cast<Person**>(Population::getDeceasedPeople()); }

	//int getInitialPopulationSize() const							{ return m_initialPopulationSize; }

	int getLastKnownPopulationSize(double &popTime) const			{ popTime = m_lastKnownPopulationSizeTime; assert(popTime >= 0); assert(m_lastKnownPopulationSize >= 0); return m_lastKnownPopulationSize; }
	void setLastKnownPopulationSize();

	double getEyeCapsFraction() const								{ return m_eyeCapsFraction; }

	// Is called by debut event
	void initializeFormationEvents(Person *pPerson);
protected:
	bool createInitialPopulation(const SimpactPopulationConfig &config, const PopulationDistribution &popDist);
	bool scheduleInitialEvents();
private:
	void onAboutToFire(EventBase *pEvt);
	void getInterestsForPerson(const Person *pPerson, std::vector<Person *> &interests);

	//int m_initialPopulationSize;
	double m_eyeCapsFraction;

	int m_lastKnownPopulationSize;
	double m_lastKnownPopulationSizeTime;

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

