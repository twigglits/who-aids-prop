#include "person.h"
#include "debugwarning.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

Person::Person(double dateOfBirth, Gender g) : PersonBase(g, dateOfBirth)
{
	m_pFather = 0;
	m_pMother = 0;

	m_lastRelationChangeTime = -1; // not set yet
	m_sexuallyActive = false;

	m_infectionTime = -1e200; // not set
	m_infected = false;
	m_pInfectionOrigin = 0;
	m_infectionType = None;
	m_acuteStage = false;

	m_breastFeeding = false;

	m_relationshipsIterator = m_relationshipsSet.begin();
#ifndef NDEBUG
	m_relIterationBusy = false;
#endif // NDEBUG
}

Person::~Person()
{
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
