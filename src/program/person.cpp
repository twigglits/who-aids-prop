#include "person.h"
#include "gslrandomnumbergenerator.h"
#include "eventtransmission.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "debugwarning.h"
#include "vspmodellogweibullwithnoise.h"
#include "vspmodellogbinormal.h"
#include "logsystem.h"
#include "simpactevent.h"
#include <stdlib.h>
#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

Person::Person(double dateOfBirth, Gender g) : PersonBase(g, dateOfBirth)
{
	m_pFather = 0;
	m_pMother = 0;

	m_lastRelationChangeTime = -1; // not set yet
	m_sexuallyActive = false;
	m_debutTime = -1;

	m_infectionTime = -1e200; // not set
	m_pInfectionOrigin = 0;
	m_infectionType = None;
	m_infectionStage = NoInfection;

	m_Vsp = 0;
	m_VspOriginal = 0;
	m_VspLowered = false;
	m_lastTreatmentStartTime = -1;
	m_treatmentCount = 0;

	m_relationshipsIterator = m_relationshipsSet.begin();
#ifndef NDEBUG
	m_relIterationBusy = false;
#endif // NDEBUG

	assert(m_pEagernessDistribution != 0);	
	m_formationEagerness = m_pEagernessDistribution->pickNumber();

	if (g == Male)
	{
		assert(m_pMaleAgeGapDistribution != 0);
		m_preferredAgeDiff = m_pMaleAgeGapDistribution->pickNumber();
	}
	else if (g == Female)
	{
		assert(m_pFemaleAgeGapDistribution != 0);
		m_preferredAgeDiff = m_pFemaleAgeGapDistribution->pickNumber();
	}
	else
		abortWithMessage("Person::Person: invalid gender type");

	m_cd4AtStart = -1;
	m_cd4AtDeath = -1;
}

Person::~Person()
{
}

void Person::setInfected(double t, Person *pOrigin, InfectionType iType)
{ 
	assert(m_infectionStage == NoInfection); 
	assert(iType != None);
	assert(!(pOrigin == 0 && iType != Seed));

	m_infectionTime = t; 
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	// Always start in the acute stage
	m_infectionStage = Acute;

	if (iType == Seed) // We need to initialize the set-point viral load
	{
		m_Vsp = pickSeedSetPointViralLoad();
		m_VspOriginal = m_Vsp;
	}
	else if (iType == Partner)
	{
		m_Vsp = pickInheritedSetPointViralLoad(pOrigin);
		m_VspOriginal = m_Vsp;
	}
	else
	{
		abortWithMessage("ERROR: unsupported infection origin!");
	}

	if (m_Vsp <= 0)
		abortWithMessage("ERROR: got invalid value for the viral load");

	m_VspLowered = false;

	// Calculate AIDS based time of death for this person
	m_aidsTodUtil.changeTimeOfDeath(t, this);

	initializeCD4Counts();
}

double Person::pickSeedSetPointViralLoad()
{
	assert(m_pVspModel != 0);
	return m_pVspModel->pickSetPointViralLoad();
}

double Person::pickInheritedSetPointViralLoad(const Person *pOrigin)
{
	assert(m_pVspModel != 0);
	double Vsp0 = pOrigin->getSetPointViralLoad();

	return m_pVspModel->inheritSetPointViralLoad(Vsp0);
}

double Person::getViralLoadFromSetPointViralLoad(double x) const
{
	assert(m_infectionStage != NoInfection);
	assert(m_Vsp > 0);
	assert(x > 0);

	double b = EventTransmission::m_b;
	double c = EventTransmission::m_c;
	double part = std::log(x)/b + std::pow(m_Vsp,-c);

	assert(m_maxViralLoad > 0);
	if (m_maxValue < 0) // we still need to calculate it
	{
		m_maxValue = std::pow(m_maxViralLoad, -c);
		assert(m_maxValue > 0);
	}

	if (c > 0)
	{
		if (part < m_maxValue)
			part = m_maxValue;
	}
	else
	{
		if (part > m_maxValue)
			part = m_maxValue;
	}

	double Vacute = std::pow(part,-1.0/c);

	return Vacute;
}

double Person::m_hivSeedWeibullShape = -1;
double Person::m_hivSeedWeibullScale = -1;
double Person::m_VspHeritabilitySigmaFraction = -1;

double Person::m_acuteFromSetPointParamX = -1;
double Person::m_aidsFromSetPointParamX = -1;
double Person::m_finalAidsFromSetPointParamX = -1;

double Person::m_maxViralLoad = -1; // this one is read from the config file
double Person::m_maxValue = -1; // this will be calculated using the value of 'c'
ProbabilityDistribution *Person::m_pEagernessDistribution = 0; // This will be a tiny memory leak
ProbabilityDistribution *Person::m_pMaleAgeGapDistribution = 0; // This will be a tiny memory leak
ProbabilityDistribution *Person::m_pFemaleAgeGapDistribution = 0; // This will be a tiny memory leak
VspModel *Person::m_pVspModel = 0;
ProbabilityDistribution *Person::m_pCD4StartDistribution = 0;
ProbabilityDistribution *Person::m_pCD4EndDistribution = 0;

void Person::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	vector<string> supportedModels;
	string VspModelName;

	supportedModels.push_back("logweibullwithnoise");
	supportedModels.push_back("logbinormal");

	if (!config.getKeyValue("person.vsp.model.type", VspModelName, supportedModels) ||
	    !config.getKeyValue("person.vsp.toacute.x", m_acuteFromSetPointParamX, 0) ||
	    !config.getKeyValue("person.vsp.toaids.x", m_aidsFromSetPointParamX, 0) ||
	    !config.getKeyValue("person.vsp.tofinalaids.x", m_finalAidsFromSetPointParamX, m_aidsFromSetPointParamX) ||
	    !config.getKeyValue("person.vsp.maxvalue", m_maxViralLoad, 0) )
		abortWithMessage(config.getErrorString());

	if (m_pVspModel != 0)
	{
		delete m_pVspModel;
		m_pVspModel = 0;
	}
	if (VspModelName == "logweibullwithnoise")
	{
		double shape, scale, fracSigma;
		vector<string> supported;
		string onNegative;

		supported.push_back("logweibull");
		supported.push_back("noiseagain");

		if (!config.getKeyValue("person.vsp.model.logweibullwithnoise.weibullscale", scale, 0) ||
		    !config.getKeyValue("person.vsp.model.logweibullwithnoise.weibullshape", shape, 0) ||
		    !config.getKeyValue("person.vsp.model.logweibullwithnoise.fracsigma", fracSigma, 0) ||
		    !config.getKeyValue("person.vsp.model.logweibullwithnoise.onnegative", onNegative, supported))
			abortWithMessage(config.getErrorString());

		VspModelLogWeibullWithRandomNoise::BadInheritType t;

		if (onNegative == "logweibull")
			t = VspModelLogWeibullWithRandomNoise::UseWeibull;
		else if (onNegative == "noiseagain")
			t = VspModelLogWeibullWithRandomNoise::NoiseAgain;
		else
			abortWithMessage("Unexpected value: " + onNegative);

		m_pVspModel = new VspModelLogWeibullWithRandomNoise(scale, shape, fracSigma, t, pRndGen);
	}
	else if (VspModelName == "logbinormal")
	{
		vector<string> yesNo;
		string alternateSeedDist;
		double mean, sigma, rho, minValue, maxValue;

		yesNo.push_back("yes");
		yesNo.push_back("no");

		if (!config.getKeyValue("person.vsp.model.logbinormal.mean", mean) ||
		    !config.getKeyValue("person.vsp.model.logbinormal.sigma", sigma, 0) ||
		    !config.getKeyValue("person.vsp.model.logbinormal.rho", rho, -1.0, 1.0) ||
		    !config.getKeyValue("person.vsp.model.logbinormal.min", minValue) ||
		    !config.getKeyValue("person.vsp.model.logbinormal.max", maxValue, minValue) || 
		    !config.getKeyValue("person.vsp.model.logbinormal.usealternativeseeddist", alternateSeedDist, yesNo)
		    )
			abortWithMessage(config.getErrorString());

		ProbabilityDistribution *pAltSeedDist = 0;
		if (alternateSeedDist == "yes")
			pAltSeedDist = getDistributionFromConfig(config, pRndGen, "person.vsp.model.logbinormal.alternativeseed");

		m_pVspModel = new VspModelLogBiNormal(mean, sigma, rho, minValue, maxValue, pAltSeedDist, pRndGen);
	}
	else
		abortWithMessage("ERROR: unexpected Vsp model name " + VspModelName);

	delete m_pEagernessDistribution;
	m_pEagernessDistribution = getDistributionFromConfig(config, pRndGen, "person.eagerness");

	delete m_pMaleAgeGapDistribution;
	m_pMaleAgeGapDistribution = getDistributionFromConfig(config, pRndGen, "person.agegap.man");

	delete m_pFemaleAgeGapDistribution;
	m_pFemaleAgeGapDistribution = getDistributionFromConfig(config, pRndGen, "person.agegap.woman");

	delete m_pCD4StartDistribution;
	m_pCD4StartDistribution = getDistributionFromConfig(config, pRndGen, "person.cd4.start");

	delete m_pCD4EndDistribution;
	m_pCD4EndDistribution = getDistributionFromConfig(config, pRndGen, "person.cd4.end");
}

void Person::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("person.vsp.toacute.x", m_acuteFromSetPointParamX) ||
	    !config.addKey("person.vsp.toaids.x", m_aidsFromSetPointParamX) ||
	    !config.addKey("person.vsp.tofinalaids.x", m_finalAidsFromSetPointParamX) ||
	    !config.addKey("person.vsp.maxvalue", m_maxViralLoad) )
		abortWithMessage(config.getErrorString());

	addDistributionToConfig(m_pEagernessDistribution, config, "person.eagerness");
	addDistributionToConfig(m_pMaleAgeGapDistribution, config, "person.agegap.man");
	addDistributionToConfig(m_pFemaleAgeGapDistribution, config, "person.agegap.woman");
	addDistributionToConfig(m_pCD4StartDistribution, config, "person.cd4.start");
	addDistributionToConfig(m_pCD4EndDistribution, config, "person.cd4.end");

	{
		VspModelLogWeibullWithRandomNoise *pDist = 0;
		if ((pDist = dynamic_cast<VspModelLogWeibullWithRandomNoise *>(m_pVspModel)) != 0)
		{
			string badInher;

			if (pDist->getOnBadInheritType() == VspModelLogWeibullWithRandomNoise::NoiseAgain)
				badInher = "noiseagain";
			else
				badInher = "logweibull";

			if (!config.addKey("person.vsp.model.type", "logweibullwithnoise") ||
		    	    !config.addKey("person.vsp.model.logweibullwithnoise.weibullshape", pDist->getWeibullShape()) ||
			    !config.addKey("person.vsp.model.logweibullwithnoise.weibullscale", pDist->getWeibullScale()) ||
		            !config.addKey("person.vsp.model.logweibullwithnoise.fracsigma", pDist->getSigmaFraction()) )
				abortWithMessage(config.getErrorString());
			    
			return;
		}
	}
	{
		VspModelLogBiNormal *pDist = 0;
		if ((pDist = dynamic_cast<VspModelLogBiNormal *>(m_pVspModel)) != 0)
		{
			ProbabilityDistribution *pAltSeedDist = pDist->getAltSeedDist();

			if (!config.addKey("person.vsp.model.type", "logbinormal") ||
			    !config.addKey("person.vsp.model.logbinormal.mean", pDist->getMean()) ||
			    !config.addKey("person.vsp.model.logbinormal.sigma", pDist->getSigma()) ||
			    !config.addKey("person.vsp.model.logbinormal.rho", pDist->getRho()) ||
			    !config.addKey("person.vsp.model.logbinormal.min", pDist->getMin()) ||
			    !config.addKey("person.vsp.model.logbinormal.max", pDist->getMax()) || 
			    !config.addKey("person.vsp.model.logbinormal.usealternativeseeddist", (pAltSeedDist != 0)))
				abortWithMessage(config.getErrorString());

			if (pAltSeedDist)
				addDistributionToConfig(pAltSeedDist, config, "person.vsp.model.logbinormal.alternativeseed");
			return;
		}
	}

	abortWithMessage("Person::obtainConfig: ERROR: unhandled Vsp model");
}

void Person::addRelationship(Person *pPerson, double t)
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

void Person::removeRelationship(Person *pPerson, double t, bool deathBased)
{
	assert(!m_relIterationBusy);
	assert(pPerson != 0);

	set<Relationship>::iterator it = m_relationshipsSet.find(pPerson);

	if (it == m_relationshipsSet.end())
		abortWithMessage("Consistency error: a person was not found exactly once in the relationship list");

	Relationship relation = *it; // save the info for logging at the end of the function

	m_relationshipsSet.erase(it);
	m_relationshipsIterator = m_relationshipsSet.begin();

	assert(t >= m_lastRelationChangeTime);
	m_lastRelationChangeTime = t;

	// Write to the event log
	
	Person *pPerson1 = 0;
	Person *pPerson2 = 0;

	if (isMan())
	{
		pPerson1 = this;
		pPerson2 = relation.getPartner();
	}
	else
	{
		pPerson1 = relation.getPartner();
		pPerson2 = this;
	}

	// TODO: is this the best approach? Only writing for the man because otherwise a log
	//       entry will appear twice (relationship is removed for two persons) unless the
	//       removal is death based (then the relationship list is only adjusted for the
	//       living person)
	if (deathBased || isMan())
	{
		SimpactEvent::writeEventLogStart(false, "(relationshipended)", t, pPerson1, pPerson2);

		double formationTime = relation.getFormationTime();
		LogEvent.print(",formationtime,%10.10f,relationage,%10.10f", formationTime, t-formationTime);

		writeToRelationLog(pPerson1, pPerson2, formationTime, t);
	}
}

void Person::writeToRelationLog(const Person *pMan, const Person *pWoman, double formationTime, double dissolutionTime)
{
	assert(pMan->isMan());
	assert(pWoman->isWoman());

	// Write to relationship log
	// male id, female id, formation time, dissolution time, age gap (age man-age woman)
	LogRelation.print("%d,%d,%10.10f,%10.10f,%10.10f", 
			  (int)pMan->getPersonID(), (int)pWoman->getPersonID(),
			  formationTime, dissolutionTime, 
			  pWoman->getDateOfBirth()-pMan->getDateOfBirth());
}

void Person::writeToPersonLog()
{
	double infinity = numeric_limits<double>::infinity();

	int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
	int gender = (isMan())?0:1;
	double timeOfBirth = getDateOfBirth();
	double timeOfDeath = (hasDied())?getTimeOfDeath():infinity;

	Man *pFather = getFather();
	Woman *pMother = getMother();
	int fatherID = (pFather != 0) ? (int)pFather->getPersonID() : (-1); // TODO: cast should be ok
	int motherID = (pMother != 0) ? (int)pMother->getPersonID() : (-1);

	// TODO: Currently not keeping track of children

	double debutTime = (isSexuallyActive())? m_debutTime:infinity;
	double formationEagerness = getFormationEagernessParameter();
	
	double infectionTime = (isInfected())? m_infectionTime : infinity;
	int origin = (m_pInfectionOrigin != 0) ? (int)m_pInfectionOrigin->getPersonID() : (-1); // TODO: cast should be ok
	
	int infectionType = 0;
	switch(m_infectionType)
	{
	case None:
		infectionType = -1;
		break;
	case Seed:
		infectionType = 0;
		break;
	case Partner:
		infectionType = 1;
		break;
	case Mother:
		infectionType = 2;
		break;
	default: // Unknown, but don't abort the program at this point
		infectionType = 10000 + (int)m_infectionType;
	}

	double log10SPVLoriginal = (isInfected()) ? std::log10(m_VspOriginal) : -infinity;
	double treatmentTime = (isInfected() && hasLoweredViralLoad()) ? getLastTreatmentStartTime() : infinity;

	LogPerson.print("%d,%d,%10.10f,%10.10f,%d,%d,%10.10f,%10.10f,%10.10f,%d,%d,%10.10f,%10.10f",
		        id, gender, timeOfBirth, timeOfDeath, fatherID, motherID, debutTime,
		        formationEagerness,
		        infectionTime, origin, infectionType, log10SPVLoriginal, treatmentTime);
}

void Person::writeToTreatmentLog(double dropoutTime, bool justDied)
{
	int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
	int gender = (isMan())?0:1;
	int justDiedInt = (justDied)?1:0;

	assert(hasLoweredViralLoad());
	assert(m_lastTreatmentStartTime >= 0);

	LogTreatment.print("%d,%d,%10.10f,%10.10f,%d", id, gender, m_lastTreatmentStartTime, dropoutTime, justDiedInt);
}

void Person::startRelationshipIteration()
{
	assert(!m_relIterationBusy);

	m_relationshipsIterator = m_relationshipsSet.begin();
#ifndef NDEBUG
	if (m_relationshipsIterator != m_relationshipsSet.end())
		m_relIterationBusy = true;
#endif
}

Person *Person::getNextRelationshipPartner(double &formationTime)
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

void Person::lowerViralLoad(double fractionOnLogscale, double treatmentTime)
{ 
	assert(m_infectionStage != NoInfection); 
	assert(m_Vsp > 0); 
	assert(!m_VspLowered); 
	assert(fractionOnLogscale > 0 && fractionOnLogscale < 1.0); 
	
	m_VspLowered = true; 
	m_Vsp = std::pow(m_Vsp, fractionOnLogscale); 
	assert(m_Vsp > 0);
	
	assert(treatmentTime >= 0); 
	m_lastTreatmentStartTime = treatmentTime;

	// This has changed the time of death
	m_aidsTodUtil.changeTimeOfDeath(treatmentTime, this);

	m_treatmentCount++;
}

void Person::resetViralLoad(double dropoutTime)
{
	assert(m_infectionStage != NoInfection); 
	assert(m_Vsp > 0 && m_VspOriginal > 0); 
	assert(m_VspLowered); 

	m_VspLowered = false;
	m_Vsp = m_VspOriginal;
	m_lastTreatmentStartTime = -1; // Not currently in treatment

	// This has changed the time of death
	m_aidsTodUtil.changeTimeOfDeath(dropoutTime, this);
}

void Person::addPersonOfInterest(Person *pPerson)
{
	assert(pPerson);
	assert(!pPerson->hasDied());
	assert(pPerson != this); // Never add ourselves

	const int num = m_personsOfInterest.size();

	for (int i = 0 ; i < num ; i++)
	{
		if (m_personsOfInterest[i] == pPerson) // Already in the list
			return;
	}

	m_personsOfInterest.push_back(pPerson);
}

void Person::removePersonOfInterest(Person *pPerson)
{
	assert(pPerson);

	const int num = m_personsOfInterest.size();

	for (int i = 0 ; i < num ; i++)
	{
		if (m_personsOfInterest[i] == pPerson)
		{
			Person *pLast = m_personsOfInterest[num-1];

			m_personsOfInterest[i] = pLast;
			m_personsOfInterest.pop_back();
			return;
		}
	}
 
	abortWithMessage("Specified person of interest " + pPerson->getName() + " was not found in list of " + getName());
}

void Person::initializeCD4Counts()
{
	assert(m_cd4AtStart < 0 && m_cd4AtDeath < 0);
	assert(m_pCD4StartDistribution && m_pCD4EndDistribution);

	m_cd4AtStart = m_pCD4StartDistribution->pickNumber();
	m_cd4AtDeath = m_pCD4EndDistribution->pickNumber();
	
	assert(m_cd4AtStart >= 0);
	assert(m_cd4AtDeath >= 0);
}

double Person::getCD4Count(double t) const
{
	// This uses a simple linear interpolation between the count at the start and at the end.
	// Once the person has been treated, this probably won't make much sense anymore, but
	// it is currently mainly a way to determine when someone should get treatment
	
	assert(m_infectionStage != NoInfection); 
	assert(m_cd4AtStart >= 0);
	assert(m_cd4AtDeath >= 0);

	double tod = m_aidsTodUtil.getTimeOfDeath();
	
	assert(t >= m_infectionTime && t <= tod);

	double frac = (t-m_infectionTime)/(tod - m_infectionTime);
	double CD4 = m_cd4AtStart + frac*(m_cd4AtDeath-m_cd4AtStart);

	return CD4;
}

Man::Man(double dateOfBirth) : Person(dateOfBirth, Male)
{
}

Man::~Man()
{
}

Woman::Woman(double dateOfBirth) : Person(dateOfBirth, Female)
{
	m_pregnant = false;
}

Woman::~Woman()
{
}

