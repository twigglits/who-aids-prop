#ifndef SIMPACTPOPULATION_H

#define SIMPACTPOPULATION_H

#include "populationinterfaces.h"
#include "person.h"
#include <assert.h>

class PopulationDistribution;
class Person;
class Man;
class Woman;

class SimpactPopulationConfig
{
public:
	SimpactPopulationConfig();
	virtual ~SimpactPopulationConfig();

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

class SimpactPopulation : public PopulationStateExtra, public PopulationAlgorithmAboutToFireInterface
{
public:
	SimpactPopulation(PopulationAlgorithmInterface &alg, PopulationStateInterface &state);
	~SimpactPopulation();

	virtual bool_t init(const SimpactPopulationConfig &popConfig, const PopulationDistribution &popDist);

	bool_t run(double &tMax, int64_t &maxEvents, double startTime = 0) { return m_alg.run(tMax, maxEvents, startTime); }

	Person **getAllPeople()						{ return reinterpret_cast<Person**>(m_state.getAllPeople()); }
	Man **getMen()								{ return reinterpret_cast<Man**>(m_state.getMen()); }
	Woman **getWomen()							{ return reinterpret_cast<Woman**>(m_state.getWomen()); }
	Person **getDeceasedPeople()				{ return reinterpret_cast<Person**>(m_state.getDeceasedPeople()); }

	int getNumberOfPeople() const				{ return m_state.getNumberOfPeople(); }
	int getNumberOfMen() const					{ return m_state.getNumberOfMen(); }
	int getNumberOfWomen() const				{ return m_state.getNumberOfWomen(); }
	int getNumberOfDeceasedPeople() const		{ return m_state.getNumberOfDeceasedPeople(); }

	void addNewPerson(Person *pPerson)			{ m_state.addNewPerson(pPerson); }
	void setPersonDied(Person *pPerson)			{ m_state.setPersonDied(pPerson); }
	void markAffectedPerson(Person *pPerson) const	{ m_state.markAffectedPerson(pPerson); }

	double getTime() const						{ return m_state.getTime(); }
	void onNewEvent(PopulationEvent *pEvt)		{ m_alg.onNewEvent(pEvt); }
	GslRandomNumberGenerator *getRandomNumberGenerator() const { return m_alg.getRandomNumberGenerator(); }

	int getLastKnownPopulationSize(double &popTime) const			{ popTime = m_lastKnownPopulationSizeTime; assert(popTime >= 0); assert(m_lastKnownPopulationSize >= 0); return m_lastKnownPopulationSize; }
	void setLastKnownPopulationSize();

	double getReferenceYear() const									{ return m_referenceYear; }
	void setReferenceYear(double t)									{ assert(t >= 0); m_referenceYear = t; }

	double getEyeCapsFraction() const								{ return m_eyeCapsFraction; }

	// Is called by debut event
	virtual void initializeFormationEvents(Person *pPerson);
protected:
	virtual bool_t createInitialPopulation(const SimpactPopulationConfig &config, const PopulationDistribution &popDist);
	virtual bool_t scheduleInitialEvents();
	virtual void getInterestsForPerson(const Person *pPerson, std::vector<Person *> &interests);
private:
	void onAboutToFire(PopulationEvent *pEvt);

	//int m_initialPopulationSize;
	double m_eyeCapsFraction;
	double m_referenceYear;

	int m_lastKnownPopulationSize;
	double m_lastKnownPopulationSizeTime;

	bool m_init;
	
	PopulationStateInterface &m_state;
	PopulationAlgorithmInterface &m_alg;
};

inline SimpactPopulation &SIMPACTPOPULATION(State *pState)
{
	assert(pState != 0);
	PopulationStateInterface &state = static_cast<PopulationStateInterface &>(*pState);
	assert(state.getExtraStateInfo() != 0);
	return static_cast<SimpactPopulation &>(*state.getExtraStateInfo());
}

inline const SimpactPopulation &SIMPACTPOPULATION(const State *pState)
{
	assert(pState != 0);
	const PopulationStateInterface &state = static_cast<const PopulationStateInterface &>(*pState);
	assert(state.getExtraStateInfo() != 0);
	return static_cast<const SimpactPopulation &>(*state.getExtraStateInfo());
}

#endif // SIMPACTPOPULATION_H

