#include "eventhivseed.h"
#include "eventaidsmortality.h"
#include "eventtransmission.h"
#include "eventchronicstage.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>
#include <cmath>

using namespace std;

EventHIVSeed::EventHIVSeed()
{
}

EventHIVSeed::~EventHIVSeed()
{
}

double EventHIVSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);

	double dt = m_seedTime - population.getTime();
	assert(m_seedTime >= 0);
	assert(dt >= 0);

	return dt;
}

string EventHIVSeed::getDescription(double tNow) const
{
	return "HIV seeding";
}

void EventHIVSeed::writeLogs(const Population &pop, double tNow) const
{
	writeEventLogStart(true, "HIV seeding", tNow, 0, 0);
}

void EventHIVSeed::fire(State *pState, double t)
{
	// Check that this event is only carried out once
	if (m_seeded)
		abortWithMessage("Can only seed population once!");

	m_seeded = true;

	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	GslRandomNumberGenerator *pRngGen = population.getRandomNumberGenerator();
	Person **ppPeople = population.getAllPeople();
	int numPeople = population.getNumberOfPeople();

	// Build pool of people which can be seeded
	assert(m_seedMinAge >= 0 && m_seedMaxAge >= m_seedMinAge);
	vector<Person *> possibleSeeders;

	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];
		double age = pPerson->getAgeAt(t);
		
		if (age >= m_seedMinAge && age <= m_seedMaxAge)
			possibleSeeders.push_back(pPerson);
	}

	// Get the actual number of people that should be seeded

	int numSeeders = 0;

	if (!m_useFraction) // we've already specified the actual number
		numSeeders = m_seedAmount;
	else
	{
		// We've specified a fraction, use a binomial distribution to obtain the number of people
		// to be seeded
		assert(m_seedFraction >= 0 && m_seedFraction <= 1.0);

		numSeeders = pRngGen->pickBinomialNumber(m_seedFraction, possibleSeeders.size());
	}

	//cout << "Poolsize = " << possibleSeeders.size() << " Num seeders = " << numSeeders << endl;

	// Mark the specified number of people as seeders
	int countSeeded = 0;
	for (int i = 0 ; i < numSeeders && possibleSeeders.size() > 0 ; i++)
	{
		int poolSize = (int)possibleSeeders.size();
		int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());

		assert(seedIdx >= 0 && seedIdx < poolSize);

		Person *pPerson = possibleSeeders[seedIdx];
		assert(!pPerson->isInfected()); // No-one should be infected before seeding!

		EventTransmission::infectPerson(population, 0, pPerson, t); // 0 means seed
		countSeeded++;

		//cout << "Seeded " << pPerson->getName() << endl;

		// remove the person from the possibleSeeders
		// To do so, we're going to move the last person to the seeder position and
		// shrink the possibleSeeders array
		Person *pLastPerson = possibleSeeders[poolSize-1];
		possibleSeeders[seedIdx] = pLastPerson;
		possibleSeeders.resize(poolSize-1);
	}

	if (!m_useFraction && m_stopOnShort && countSeeded != m_seedAmount)
		abortWithMessage(strprintf("Could not seed the requested amount of people: %d were seeded, but %d requested", countSeeded, m_seedAmount));
}

double EventHIVSeed::m_seedTime = -1;
double EventHIVSeed::m_seedFraction = -1;
double EventHIVSeed::m_seedMinAge = -1;
double EventHIVSeed::m_seedMaxAge = -1;
int EventHIVSeed::m_seedAmount = -1;
bool EventHIVSeed::m_stopOnShort = false;
bool EventHIVSeed::m_useFraction = true;
bool EventHIVSeed::m_seeded = false;

void EventHIVSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	vector<string> fractionOrAmountStrings;
	string fractionOrAmount;

	fractionOrAmountStrings.push_back("fraction");
	fractionOrAmountStrings.push_back("amount");

	if (!config.getKeyValue("hivseed.time", m_seedTime) ||
	    !config.getKeyValue("hivseed.type", fractionOrAmount, fractionOrAmountStrings) ||
	    !config.getKeyValue("hivseed.age.min", m_seedMinAge, 0) ||
	    !config.getKeyValue("hivseed.age.max", m_seedMaxAge, m_seedMinAge)
	    )
		abortWithMessage(config.getErrorString());

	if (fractionOrAmount == "fraction")
	{
		m_useFraction = true;

		if (!config.getKeyValue("hivseed.fraction", m_seedFraction, 0, 1))
			abortWithMessage(config.getErrorString());
	}
	else if (fractionOrAmount == "amount")
	{
		m_useFraction = false;

		if (!config.getKeyValue("hivseed.amount", m_seedAmount, 0) ||
		    !config.getKeyValue("hivseed.stop.short", m_stopOnShort)
		    )
			abortWithMessage(config.getErrorString());
	}
	else
		abortWithMessage("Internal error: unexpected 'hivseed.type'");
}

void EventHIVSeed::obtainConfig(ConfigWriter &config)
{
	string seedType;

	if (m_useFraction)
		seedType = "fraction";
	else
		seedType = "amount";

	if (!config.addKey("hivseed.time", m_seedTime) ||
	    !config.addKey("hivseed.type", seedType) ||
	    !config.addKey("hivseed.age.min", m_seedMinAge) ||
	    !config.addKey("hivseed.age.max", m_seedMaxAge)
	    )
		abortWithMessage(config.getErrorString());

	if (m_useFraction)
	{
		if (!config.addKey("hivseed.fraction", m_seedFraction))
			abortWithMessage(config.getErrorString());
	}
	else
	{
		if (!config.addKey("hivseed.amount", m_seedAmount) ||
		    !config.addKey("hivseed.stop.short", m_stopOnShort) )
			abortWithMessage(config.getErrorString());
	}
}

ConfigFunctions hivseedingConfigFunctions(EventHIVSeed::processConfig, EventHIVSeed::obtainConfig, "EventHIVSeed");

JSONConfig hivseedingJSONConfig(R"JSON(
        "EventSeeding": { 
            "depends": null, 
            "params": [ 
                ["hivseed.time", 0],
                ["hivseed.type", "fraction", [ "fraction", "amount"] ],
                ["hivseed.age.min", 0],
                ["hivseed.age.max", 1000]
            ],
            "info": [ 
                "Controls when the initial HIV seeders are introduced, and who those seeders",
                "are. First, the possible seeders are chosen from the population, based on the",
                "specified mininum and maximum ages.",
                "",
                "The specified time says when the seeding event should take place. Note that",
                "if the time is negative, no seeders will be introduced since the event will ",
                "be ignored (simulation time starts at t = 0)."
            ]
        },

        "EventSeeding_fraction": {
            "depends": [ "EventSeeding", "hivseed.type", "fraction" ],
            "params": [
                [ "hivseed.fraction", 0.2 ]
            ],
            "info": [
                "From the people who possibly can be seeded with HIV, the specified fraction",
                "will be marked as infected."
            ]
        },

        "EventSeeding_number": {
            "depends": [ "EventSeeding", "hivseed.type", "amount" ],
            "params": [
                [ "hivseed.amount", 1 ],
                [ "hivseed.stop.short", "yes", [ "yes", "no" ] ]
            ],
            "info": [
                "From the people who possibly can be seeded with HIV, the specified amount",
                "will be marked as infected. If the 'hivseed.stop.short' parameter is set to",
                "'yes' and there were not enough people that could be infected, the program",
                "will abort."
            ]
        })JSON");

