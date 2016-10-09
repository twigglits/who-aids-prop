#ifndef PERSON_H

#define PERSON_H

#include "personbase.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>

class Person;
class Man;
class Woman;

Man *MAN(Person *pPerson);
Woman *WOMAN(Person *pPerson);

class Person : public PersonBase
{
public:
	enum InfectionType { None, Partner, Mother, Seed };

	Person(double dateOfBirth, Gender g);
	~Person();

	bool isMan() const								{ return getGender() == Male; }
	bool isWoman() const								{ return getGender() == Female; }

	void setFather(Man *pFather)							{ assert(m_pFather == 0); assert(pFather != 0); m_pFather = pFather; }
	void setMother(Woman *pMother)							{ assert(m_pMother == 0); assert(pMother != 0);  m_pMother = pMother; }

	Man *getFather() const								{ return m_pFather; }
	Woman *getMother() const							{ return m_pMother; }

	// This also resets the iterator for getNextRelationshipPartner
	int getNumberOfRelationships() const						{ return m_relationshipsSet.size(); }
	void startRelationshipIteration();
	Person *getNextRelationshipPartner(double &formationTime);

	bool hasRelationshipWith(Person *pPerson) const;

	// WARNING: do not use these during relationship iteration
	void addRelationship(Person *pPerson, double t);
	void removeRelationship(Person *pPerson, double t);
	
	// result is negative if no relations formed yet
	double getLastRelationshipChangeTime() const					{ return m_lastRelationChangeTime; }

	void setSexuallyActive()							{ m_sexuallyActive = true; }
	bool isSexuallyActive()								{ return m_sexuallyActive; }

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	bool isInfected() const								{ return m_infected; }
	double getInfectionTime() const							{ assert(m_infected); return m_infectionTime; }
	bool inAcuteStage() const							{ assert(m_infected); return m_acuteStage; }
	void setInChronicStage()							{ assert(m_infected); assert(m_acuteStage); m_acuteStage = false; }

	// TODO: currently, the death of a parent or the death of a child does
	// not have any influence on this list. I don't think modifying the list
	// on a mortality event is useful, will only complicate things
	// TODO: what might be useful is a member function to retrieve the number
	// of living children?
	void addChild(Person *pPerson);
	bool hasChild(Person *pPerson) const;
	int getNumberOfChildren() const							{ return m_children.size(); }

	void setBreastFeeding()								{ assert(!m_breastFeeding); m_breastFeeding = true; }
	bool isBreastFeeding() const							{ return m_breastFeeding; }
	void stopBreastFeeding()							{ assert(m_breastFeeding); m_breastFeeding = false; }
private:
	class Relationship
	{
	public:
		// A negative time is used when not relevant, e.g. when
		// searching for a relationship with a person
		Relationship(Person *pPerson, double formationTime)				{ assert(pPerson != 0); assert(formationTime > 0); m_pPerson = pPerson; m_formationTime = formationTime; }
		Relationship(Person *pPerson)							{ assert(pPerson != 0); m_pPerson = pPerson; m_formationTime = -1; }

		Person *getPartner() const							{ return m_pPerson; }
		double getFormationTime() const							{ return m_formationTime; }

		bool operator<(const Relationship &rel) const
		{
			if (m_pPerson->getPersonID() < rel.m_pPerson->getPersonID()) 
				return true; 
			return false; 
		}
	private:
		Person *m_pPerson;
		double m_formationTime;
	};

	std::set<Relationship> m_relationshipsSet;
	std::set<Relationship>::const_iterator m_relationshipsIterator;
	double m_lastRelationChangeTime;
	bool m_sexuallyActive;

	double m_infectionTime;
	bool m_infected;
	Person *m_pInfectionOrigin;
	InfectionType m_infectionType;
	bool m_acuteStage;

	Man *m_pFather;
	Woman *m_pMother;

	std::vector<Person *> m_children;

	bool m_breastFeeding;

#ifndef NDEBUG
	bool m_relIterationBusy;
#endif
};

class Man : public Person
{
public:
	Man(double dateOfBirth);
	~Man();
};

class Woman : public Person
{
public:
	Woman(double dateOfBirth);
	~Woman();

	void setPregnant(bool f)							{ m_pregnant = f; }
	bool isPregnant() const								{ return m_pregnant; }
private:
	bool m_pregnant;
};

inline void Person::addRelationship(Person *pPerson, double t)
{
	assert(!m_relIterationBusy);
	assert(pPerson != 0);
	assert(pPerson != this); // can't have a relationship with ourselves
	assert(!hasDied() && !pPerson->hasDied());
	// Check that the relationship doesn't exist yet (debug mode only)
	assert(m_relationshipsSet.find(Person::Relationship(pPerson)) == m_relationshipsSet.end());

	Person::Relationship r(pPerson, t);

	m_relationshipsSet.insert(r);
	m_relationshipsIterator = m_relationshipsSet.begin();

	assert(t >= m_lastRelationChangeTime);
	m_lastRelationChangeTime = t;
}

inline void Person::removeRelationship(Person *pPerson, double t)
{
	assert(!m_relIterationBusy);
	assert(pPerson != 0);

	std::set<Person::Relationship>::iterator it = m_relationshipsSet.find(pPerson);

	if (it == m_relationshipsSet.end())
	{
		std::cerr << "Consistency error: a person was not found exactly once in the relationship list" << std::endl;
		exit(-1);
	}

	m_relationshipsSet.erase(it);
	m_relationshipsIterator = m_relationshipsSet.begin();

	assert(t >= m_lastRelationChangeTime);
	m_lastRelationChangeTime = t; 
}

inline bool Person::hasRelationshipWith(Person *pPerson) const
{
	return m_relationshipsSet.find(Person::Relationship(pPerson)) != m_relationshipsSet.end();
}

inline void Person::addChild(Person *pPerson)
{
	assert(pPerson != 0);
	assert(!hasChild(pPerson));

	m_children.push_back(pPerson);
}

// TODO: this is currently not fast for large number of children
//       can always use a 'set' if this becomes a bottleneck
inline bool Person::hasChild(Person *pPerson) const
{
	assert(pPerson != 0);

	for (int i = 0 ; i < m_children.size() ; i++)
	{
		assert(m_children[i] != 0);
		
		if (m_children[i] == pPerson)
			return true;
	}

	return false;
}

inline void Person::setInfected(double t, Person *pOrigin, InfectionType iType)
{ 
	assert(!m_infected); 
	m_infected = true; 
	
	m_infectionTime = t; 
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	m_acuteStage = true;

	assert(iType != None);
	assert(!(pOrigin == 0 && iType != Seed));
}

inline void Person::startRelationshipIteration()
{
	assert(!m_relIterationBusy);

	m_relationshipsIterator = m_relationshipsSet.begin();
#ifndef NDEBUG
	if (m_relationshipsIterator != m_relationshipsSet.end())
		m_relIterationBusy = true;
#endif
}

inline Person *Person::getNextRelationshipPartner(double &formationTime)
{
	if (m_relationshipsIterator == m_relationshipsSet.end())
	{
#ifndef NDEBUG
		m_relIterationBusy = false;
#endif
		return 0;
	}

	assert(m_relIterationBusy);

	const Person::Relationship &r = *m_relationshipsIterator;
	
	++m_relationshipsIterator;

	formationTime = r.getFormationTime();
	return r.getPartner();
}

inline Man *MAN(Person *pPerson)
{
	assert(pPerson != 0);
	assert(pPerson->getGender() == PersonBase::Male);

	return static_cast<Man*>(pPerson);
}

inline Woman *WOMAN(Person *pPerson)
{
	assert(pPerson != 0);
	assert(pPerson->getGender() == PersonBase::Female);

	return static_cast<Woman*>(pPerson);
}

#endif // PERSON_H

