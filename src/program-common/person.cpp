#include "person.h"
#include "personimpl.h"
#include "gslrandomnumbergenerator.h"
#include "eventtransmission.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "debugwarning.h"
#include "vspmodellogweibullwithnoise.h"
#include "vspmodellogdist.h"
#include "logsystem.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include "tiffdensityfile.h"
#include "jsonconfig.h"
#include "configfunctions.h"
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
	m_diagnoseCount = 0;

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

	assert(m_pARTAcceptDistribution);
	m_artAcceptanceThreshold = m_pARTAcceptDistribution->pickNumber();

	assert(m_pPopDist);

	m_location = m_pPopDist->pickPoint();
	assert(m_location.x == m_location.x && m_location.y == m_location.y);

	m_aidsDeath = false;

	assert(m_pLogSurvTimeOffsetDistribution);
	m_log10SurvTimeOffset = m_pLogSurvTimeOffsetDistribution->pickNumber();

	m_pPersonImpl = new PersonImpl(*this);
}

Person::~Person()
{
	delete m_pPersonImpl;
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

	double b = EventTransmission::getParamB();
	double c = EventTransmission::getParamC();
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
ProbabilityDistribution *Person::m_pARTAcceptDistribution = 0;

ProbabilityDistribution2D *Person::m_pPopDist = 0;
double Person::m_popDistWidth = 0;
double Person::m_popDistHeight = 0;

ProbabilityDistribution *Person::m_pLogSurvTimeOffsetDistribution = 0;

void Person::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	vector<string> supportedModels;
	string VspModelName;

	supportedModels.push_back("logweibullwithnoise");
	supportedModels.push_back("logdist2d");

	bool_t r;

	if (!(r = config.getKeyValue("person.vsp.model.type", VspModelName, supportedModels)) ||
	    !(r = config.getKeyValue("person.vsp.toacute.x", m_acuteFromSetPointParamX, 0)) ||
	    !(r = config.getKeyValue("person.vsp.toaids.x", m_aidsFromSetPointParamX, 0)) ||
	    !(r = config.getKeyValue("person.vsp.tofinalaids.x", m_finalAidsFromSetPointParamX, m_aidsFromSetPointParamX)) ||
	    !(r = config.getKeyValue("person.vsp.maxvalue", m_maxViralLoad, 0)) )
		abortWithMessage(r.getErrorString());

	delete m_pVspModel;
	m_pVspModel = 0;

	if (VspModelName == "logweibullwithnoise")
	{
		double shape, scale, fracSigma;
		vector<string> supported;
		string onNegative;

		supported.push_back("logweibull");
		supported.push_back("noiseagain");

		if (!(r = config.getKeyValue("person.vsp.model.logweibullwithnoise.weibullscale", scale, 0)) ||
		    !(r = config.getKeyValue("person.vsp.model.logweibullwithnoise.weibullshape", shape, 0)) ||
		    !(r = config.getKeyValue("person.vsp.model.logweibullwithnoise.fracsigma", fracSigma, 0)) ||
		    !(r = config.getKeyValue("person.vsp.model.logweibullwithnoise.onnegative", onNegative, supported)))
			abortWithMessage(r.getErrorString());

		VspModelLogWeibullWithRandomNoise::BadInheritType t;

		if (onNegative == "logweibull")
			t = VspModelLogWeibullWithRandomNoise::UseWeibull;
		else if (onNegative == "noiseagain")
			t = VspModelLogWeibullWithRandomNoise::NoiseAgain;
		else
			abortWithMessage("Unexpected value: " + onNegative);

		m_pVspModel = new VspModelLogWeibullWithRandomNoise(scale, shape, fracSigma, t, pRndGen);
	}
	else if (VspModelName == "logdist2d")
	{
		bool useAltSeedDist;

		if (!(r = config.getKeyValue("person.vsp.model.logdist2d.usealternativeseeddist", useAltSeedDist)))
			abortWithMessage(r.getErrorString());

		ProbabilityDistribution2D *pDist2D = 0;
		ProbabilityDistribution *pAltSeedDist = 0;
		
		if (useAltSeedDist)
			pAltSeedDist = getDistributionFromConfig(config, pRndGen, "person.vsp.model.logdist2d.alternativeseed");

		pDist2D = getDistribution2DFromConfig(config, pRndGen, "person.vsp.model.logdist2d");

		m_pVspModel = new VspModelLogDist(pDist2D, pAltSeedDist, pRndGen);
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

	delete m_pARTAcceptDistribution;
	m_pARTAcceptDistribution = getDistributionFromConfig(config, pRndGen, "person.art.accept.threshold");

	delete m_pLogSurvTimeOffsetDistribution;
	m_pLogSurvTimeOffsetDistribution = getDistributionFromConfig(config, pRndGen, "person.survtime.logoffset");

	// Population distribution
	delete m_pPopDist;
	m_pPopDist = getDistribution2DFromConfig(config, pRndGen, "person.geo");
}

void Person::obtainConfig(ConfigWriter &config)
{
	assert(m_pPopDist);
	addDistribution2DToConfig(m_pPopDist, config, "person.geo");

	bool_t r;

	if (!(r = config.addKey("person.vsp.toacute.x", m_acuteFromSetPointParamX)) ||
	    !(r = config.addKey("person.vsp.toaids.x", m_aidsFromSetPointParamX)) ||
	    !(r = config.addKey("person.vsp.tofinalaids.x", m_finalAidsFromSetPointParamX)) ||
	    !(r = config.addKey("person.vsp.maxvalue", m_maxViralLoad)) )
		abortWithMessage(r.getErrorString());

	addDistributionToConfig(m_pEagernessDistribution, config, "person.eagerness");
	addDistributionToConfig(m_pMaleAgeGapDistribution, config, "person.agegap.man");
	addDistributionToConfig(m_pFemaleAgeGapDistribution, config, "person.agegap.woman");
	addDistributionToConfig(m_pCD4StartDistribution, config, "person.cd4.start");
	addDistributionToConfig(m_pCD4EndDistribution, config, "person.cd4.end");
	addDistributionToConfig(m_pARTAcceptDistribution, config, "person.art.accept.threshold");
	addDistributionToConfig(m_pLogSurvTimeOffsetDistribution, config, "person.survtime.logoffset");

	{
		VspModelLogWeibullWithRandomNoise *pDist = 0;
		if ((pDist = dynamic_cast<VspModelLogWeibullWithRandomNoise *>(m_pVspModel)) != 0)
		{
			string badInher;

			if (pDist->getOnBadInheritType() == VspModelLogWeibullWithRandomNoise::NoiseAgain)
				badInher = "noiseagain";
			else
				badInher = "logweibull";

			if (!(r = config.addKey("person.vsp.model.type", "logweibullwithnoise")) ||
		    	!(r = config.addKey("person.vsp.model.logweibullwithnoise.weibullshape", pDist->getWeibullShape())) ||
			    !(r = config.addKey("person.vsp.model.logweibullwithnoise.weibullscale", pDist->getWeibullScale())) ||
		        !(r = config.addKey("person.vsp.model.logweibullwithnoise.fracsigma", pDist->getSigmaFraction())) )
				abortWithMessage(r.getErrorString());
			    
			return;
		}
	}
	{
		VspModelLogDist *pDist = 0;
		if ((pDist = dynamic_cast<VspModelLogDist *>(m_pVspModel)) != 0)
		{
			ProbabilityDistribution *pAltSeedDist = pDist->getAltSeedDist();
			ProbabilityDistribution2D *pDist2D = pDist->getUnderlyingDistribution();

			if (!(r = config.addKey("person.vsp.model.type", "logdist2d")) ||
			    !(r = config.addKey("person.vsp.model.logdist2d.usealternativeseeddist", (pAltSeedDist != 0))))
				abortWithMessage(r.getErrorString());

			if (pAltSeedDist)
				addDistributionToConfig(pAltSeedDist, config, "person.vsp.model.logdist2d.alternativeseed");

			addDistribution2DToConfig(pDist2D, config, "person.vsp.model.logdist2d");
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

int Person::getNumberOfDiagnosedPartners()
{
	// IMPORTANT: for a simple method, we cannot cache the result, it will
	//            not only change on relationship events, but also on transmission

	int D = 0; // number of diagnosed partners
	int numRelations = getNumberOfRelationships();
	double tDummy = 0;

	// TODO: is this a bottleneck? Should it be faster?
	startRelationshipIteration();
	for (int i = 0 ; i < numRelations ; i++)
	{
		Person *pPartner = getNextRelationshipPartner(tDummy);
		if (pPartner->isDiagnosed())
			D++;
	}
	assert(getNextRelationshipPartner(tDummy) == 0);

	return D;
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

	int aidsDeath = -1;
	if (hasDied())
	{
		aidsDeath = 0;
		if (m_aidsDeath)
			aidsDeath = 1;
	}
	LogPerson.print("%d,%d,%10.10f,%10.10f,%d,%d,%10.10f,%10.10f,%10.10f,%d,%d,%10.10f,%10.10f,%10.10f,%10.10f,%d",
		        id, gender, timeOfBirth, timeOfDeath, fatherID, motherID, debutTime,
		        formationEagerness,
		        infectionTime, origin, infectionType, log10SPVLoriginal, treatmentTime,
			m_location.x, m_location.y, aidsDeath);
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

ConfigFunctions personConfigFunctions(Person::processConfig, Person::obtainConfig, "Person");

JSONConfig personJSONConfig(R"JSON(
        "PersonVspAcute": { 
            "depends": null,
            "params": [ 
                ["person.vsp.toacute.x", 10.0],
                ["person.vsp.toaids.x", 7.0],
                ["person.vsp.tofinalaids.x", 12.0],
                ["person.vsp.maxvalue", 1e9] ],
            "info": [ 
                "The viral load during the other stages is based on the set point viral load:",
                "   V = [ max(ln(x)/b + Vsp^(-c), maxvalue^(-c)) ]^(-1/c)",
                "The b and c parameters are specified in the parameters from the transmission",
                "event."
            ]
        },

        "PersonCD4": {
            "depends": null,
            "params": [ 
                [ "person.cd4.start.dist", "distTypes", [ "uniform", [ [ "min", 700  ], [ "max", 1300 ] ] ] ],
                [ "person.cd4.end.dist", "distTypes", [ "uniform", [ [ "min", 0  ], [ "max", 100 ] ] ] ]
            ],
            "info": [
                "These distributions control the initial CD4 count when first getting infected",
                "and the final CD4 count at the time the person dies from AIDS."
            ]
        },

        "PersonEagerness": { 
            "depends": null, 
            "params": [ ["person.eagerness.dist", "distTypes" ] ],
            "info": [ 
                "The per-person parameter for the eagerness to form a relationship is chosen",
                "from a specific distribution with certain parameters."
            ]
        },

        "PersonAgeGapMen": { 
            "depends": null, 
            "params": [ ["person.agegap.man.dist", "distTypes" ] ],
            "info": null
        },

        "PersonAgeGapWomen": { 
            "depends": null, 
            "params": [ ["person.agegap.woman.dist", "distTypes" ] ],
            "info": null
        },

        "PersonVspModelTypes": { 
            "depends": null, 
            "params": [ ["person.vsp.model.type", "logdist2d", [ "logweibullwithnoise", "logdist2d"] ] ],
            "info": [ 
                "The type of model to use for the Vsp value of the seeders and for inheriting",
                "Vsp values."
            ]
        },

        "PersonVspModel_weibullnoise": { 
            "depends": ["PersonVspModelTypes", "person.vsp.model.type", "logweibullwithnoise"],
            "params": [ 
                ["person.vsp.model.logweibullwithnoise.weibullscale", 5.05],
                ["person.vsp.model.logweibullwithnoise.weibullshape", 7.2],
                ["person.vsp.model.logweibullwithnoise.fracsigma", 0.1],
                ["person.vsp.model.logweibullwithnoise.onnegative", "logweibull", [ "logweibull", "noiseagain"] ] 
            ],
            "info": [ 
                "For 'seeders', people marked as infected at the start of the simulation,",
                "the logarithm of set-point viral load is chosen from a weibull distribution.",
                "",
                "In Vsp heritability, added random noise uses a sigma that's 10% of the ",
                "original Vsp. When after adding noise upon inheriting the Vsp value, the",
                "Vsp is negative: use 'noiseagain' to pick from gaussian(VspOrigin,sigma) ",
                "again, or use 'logweibull' to pick from the initial distribution again."
            ]
        },

        "PersonVspModel_logdist": { 
            "depends": ["PersonVspModelTypes", "person.vsp.model.type", "logdist2d"],
            "params": [ 
                ["person.vsp.model.logdist2d.dist2d", "distTypes2D", [ 
                    "binormalsymm", [
                        [ "min", 1 ],
                        [ "max", 8 ],
                        [ "mean", 4 ],
                        [ "rho", 0.33 ],
                        [ "sigma", 1 ]
                        ]
                    ]
                ],
                ["person.vsp.model.logdist2d.usealternativeseeddist", "no", [ "yes", "no"]]
            ],
            "info": [ 
                "Both the initial 'seed' value and the inherited Vsp value are",
                "chosen so that the log value is based on the specified 2D distribution.",
                "",
                "Additionally, you can also specify that an alternative distribution must",
                "be used to pick the Vsp values of the seeders."
            ]                                         
        },

        "PersonVspModel_logdist_altseed": { 
            "depends": ["PersonVspModel_logdist", "person.vsp.model.logdist2d.usealternativeseeddist", "yes"],
            "params": [ ["person.vsp.model.logdist2d.alternativeseed.dist", "distTypes" ] ],
            "info": null 
        },

        "PersonARTAcceptange": {
            "depends": null,
            "params": [ 
                [ "person.art.accept.threshold.dist", "distTypes", ["fixed", [ ["value", 0.5 ] ] ] ]
            ],
            "info": [
                "This parameter specifies a distribution from which a number will be chosen",
                "for each person, and which serves as the threshold to start ART (if eligible).",
                "When eligible for treatment, a random number will be chosen uniformly from",
                "[0,1], and treatment will only be started if this number is smaller than the",
                "threshold. By default, everyone will just have a 50/50 chance of starting",
                "treatment when possible. ",
                "",
                "If this distribution returns a low value (close to zero), it means that ",
                "there's little chance of accepting treatment; if the value is higher (close to",
                "one), treatment will almost always be accepted."
            ]
        },

        "PersonSurvTimeLogOffset": {
            "depends": null,
            "params": [
                [ "person.survtime.logoffset.dist", "distTypes" ]
            ],
            "info": [
                "By configuring this, you can add an offset to the survival time that differs",
                "per person, so the relationship between survival time and viral load will",
                "show some scatter. The offset is added to the logarithm of the survival time."
            ]
        })JSON");


