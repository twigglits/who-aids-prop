#ifndef PERSON_H

#define PERSON_H

#include "personbase.h"
#include "probabilitydistribution2d.h"
#include "util.h"
#include "aidstodutil.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>

class PersonImpl;
class Person;
class Man;
class Woman;
class GslRandomNumberGenerator;
class ConfigSettings;
class ConfigWriter;
class DiscreteDistribution2D;
class ProbabilityDistribution;
class VspModel;

Man *MAN(Person *pPerson);
Woman *WOMAN(Person *pPerson);

class Person : public PersonBase
{
public:
	enum InfectionType { None, Partner, Mother, Seed };
	enum InfectionStage { NoInfection, Acute, Chronic, AIDS, AIDSFinal };

	Person(double dateOfBirth, Gender g);
	~Person();

	PersonImpl *getImplementationSpecificPart()					{ return m_pPersonImpl; }

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
	int getNumberOfDiagnosedPartners();

	bool hasRelationshipWith(Person *pPerson) const;

	// WARNING: do not use these during relationship iteration
	void addRelationship(Person *pPerson, double t);
	void removeRelationship(Person *pPerson, double t, bool deathBased);
	
	// result is negative if no relations formed yet
	double getLastRelationshipChangeTime() const					{ return m_lastRelationChangeTime; }

	void setSexuallyActive(double t)						{ m_sexuallyActive = true; assert(t >= 0); m_debutTime = t; }
	bool isSexuallyActive()								{ return m_sexuallyActive;}
	double getDebutTime() const							{ return m_debutTime; }

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	bool isInfected() const								{ if (m_infectionStage == NoInfection) return false; return true; }
	double getInfectionTime() const							{ assert(m_infectionStage != NoInfection); return m_infectionTime; }
	InfectionStage getInfectionStage() const					{ return m_infectionStage; }
	void setInChronicStage()							{ assert(m_infectionStage == Acute); m_infectionStage = Chronic; }
	void setInAIDSStage()								{ assert(m_infectionStage == Chronic); m_infectionStage = AIDS; }
	void setInFinalAIDSStage()							{ assert(m_infectionStage == AIDS); m_infectionStage = AIDSFinal; }
	double getAIDSMortalityTime() const						{ return m_aidsTodUtil.getTimeOfDeath(); }

	bool isDiagnosed() const							{ return (m_diagnoseCount > 0); }
	void increaseDiagnoseCount()							{ m_diagnoseCount++; }
	int getDiagnoseCount() const							{ return m_diagnoseCount; }

	double getSetPointViralLoad() const						{ assert(m_infectionStage != NoInfection); return m_Vsp; }
	double getViralLoad() const;
	void lowerViralLoad(double fractionOnLogscale, double treatmentTime);
	bool hasLoweredViralLoad() const						{ assert(isInfected()); assert(m_Vsp > 0); return m_VspLowered; }
	double getLastTreatmentStartTime() const					{ assert(isInfected()); assert(m_Vsp > 0); assert(m_VspLowered); assert(m_lastTreatmentStartTime >= 0); return m_lastTreatmentStartTime; }
	void resetViralLoad(double dropoutTime);
	int getNumberTreatmentStarted() const						{ assert(isInfected()); return m_treatmentCount; }

	double getFormationEagernessParameter() const					{ return m_formationEagerness; }
	double getPreferredAgeDifference() const					{ assert(m_preferredAgeDiff < 200.0 && m_preferredAgeDiff > -200.0); return m_preferredAgeDiff; }

	// TODO: currently, the death of a parent or the death of a child does
	// not have any influence on this list. I don't think modifying the list
	// on a mortality event is useful, will only complicate things
	// TODO: what might be useful is a member function to retrieve the number
	// of living children?
	void addChild(Person *pPerson);
	bool hasChild(Person *pPerson) const;
	int getNumberOfChildren() const							{ return m_children.size(); }
	Person *getChild(int idx);

	// NOTE: this ignores the call if already in the list
	void addPersonOfInterest(Person *pPerson);
	void removePersonOfInterest(Person *pPerson);
	void clearPersonsOfInterest()								{ m_personsOfInterest.clear(); }
	int getNumberOfPersonsOfInterest() const						{ return (int)m_personsOfInterest.size(); }
	Person *getPersonOfInterest(int idx) const						{ assert(idx >= 0 && idx < (int)m_personsOfInterest.size()); Person *pPerson = m_personsOfInterest[idx]; assert(pPerson); return pPerson; }

	double getCD4Count(double t) const;
	double getARTAcceptanceThreshold() const						{ return m_artAcceptanceThreshold; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);

	static void writeToRelationLog(const Person *pMan, const Person *pWoman, double formationTime, double dissolutionTime);
	void writeToPersonLog();
	void writeToTreatmentLog(double dropoutTime, bool justDied);

	Point2D getLocation() const															{ return m_location; }
	void markAIDSDeath()									{ assert(hasDied()); m_aidsDeath = true; }
	bool wasAIDSDeath() const								{ assert(hasDied()); return m_aidsDeath; }

	// This is a per person value
	double getSurvivalTimeLog10Offset() const						{ return m_log10SurvTimeOffset; }
private:
	double initializeEagerness();
	double getViralLoadFromSetPointViralLoad(double x) const;
	void initializeCD4Counts();
	static double pickSeedSetPointViralLoad();
	static double pickInheritedSetPointViralLoad(const Person *pOrigin);

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

#ifndef NDEBUG
	bool m_relIterationBusy;
#endif

	std::set<Relationship> m_relationshipsSet;
	std::set<Relationship>::const_iterator m_relationshipsIterator;
	double m_lastRelationChangeTime;
	bool m_sexuallyActive;
	double m_debutTime;

	double m_infectionTime;
	Person *m_pInfectionOrigin;
	InfectionType m_infectionType;
	InfectionStage m_infectionStage;
	int m_diagnoseCount;
	bool m_aidsDeath;
	double m_log10SurvTimeOffset;

	double m_Vsp, m_VspOriginal;
	bool m_VspLowered;
	double m_lastTreatmentStartTime;
	int m_treatmentCount;

	Man *m_pFather;
	Woman *m_pMother;

	double m_formationEagerness;
	double m_preferredAgeDiff;

	std::vector<Person *> m_children;
	std::vector<Person *> m_personsOfInterest;

	Point2D m_location;

	AIDSTimeOfDeathUtility m_aidsTodUtil;

	double m_cd4AtStart, m_cd4AtDeath;
	double m_artAcceptanceThreshold;

	PersonImpl *m_pPersonImpl;

	static double m_hivSeedWeibullShape;
	static double m_hivSeedWeibullScale;
	static double m_VspHeritabilitySigmaFraction;
	static double m_acuteFromSetPointParamX;
	static double m_aidsFromSetPointParamX;
	static double m_finalAidsFromSetPointParamX;
	static double m_maxValue;
	static double m_maxViralLoad;
	static ProbabilityDistribution *m_pEagernessDistribution;
	static ProbabilityDistribution *m_pMaleAgeGapDistribution;
	static ProbabilityDistribution *m_pFemaleAgeGapDistribution;
	static VspModel *m_pVspModel;
	static ProbabilityDistribution2D *m_pPopDist;
	static double m_popDistWidth;
	static double m_popDistHeight;
	static ProbabilityDistribution *m_pCD4StartDistribution;
	static ProbabilityDistribution *m_pCD4EndDistribution;
	static ProbabilityDistribution *m_pARTAcceptDistribution;
	static ProbabilityDistribution *m_pLogSurvTimeOffsetDistribution;
};

inline double Person::getViralLoad() const
{ 
	assert(m_infectionStage != NoInfection); 
	if (m_infectionStage == Acute) 
		return getViralLoadFromSetPointViralLoad(m_acuteFromSetPointParamX); 
	else if (m_infectionStage == Chronic)
		return getSetPointViralLoad(); 
	else if (m_infectionStage == AIDS)
		return getViralLoadFromSetPointViralLoad(m_aidsFromSetPointParamX);
	else if (m_infectionStage == AIDSFinal)
		return getViralLoadFromSetPointViralLoad(m_finalAidsFromSetPointParamX);
	
	abortWithMessage("Unknown stage in Person::getViralLoad");
	return -1;
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

	for (size_t i = 0 ; i < m_children.size() ; i++)
	{
		assert(m_children[i] != 0);
		
		if (m_children[i] == pPerson)
			return true;
	}

	return false;
}

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

inline bool Person::hasRelationshipWith(Person *pPerson) const
{
	return m_relationshipsSet.find(Person::Relationship(pPerson)) != m_relationshipsSet.end();
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

inline Person* Person::getChild(int idx)
{ 
	assert(idx >= 0 && idx < (int)m_children.size()); 
	Person *pChild = m_children[idx]; 

	assert(pChild != 0); 
#ifndef NDEBUG
	if (isWoman())
	{
		assert(pChild->getMother() == this);
	}
	else // we should be the father of the child
	{
		assert(pChild->getFather() == this);
	}
#endif
	return pChild; 
}

#endif // PERSON_H

