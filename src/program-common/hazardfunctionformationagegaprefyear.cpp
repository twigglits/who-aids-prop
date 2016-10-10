#include "hazardfunctionformationagegaprefyear.h"
#include "person.h"
#include "eventdebut.h"
#include <iostream>
#include <assert.h>

using namespace std;

HazardFunctionFormationAgeGapRefYear::HazardFunctionFormationAgeGapRefYear(const Person *pPerson1, const Person *pPerson2, 
                   double tr,
                   double a0, double a1, double a2, double a3, double a4, 
				   double a8, double a10, 
				   double agfmConst, double agfmExp, double agfmAge,
				   double agfwConst, double agfwExp, double agfwAge,
				   double b,
				   double ageRefYear)
{
	assert(pPerson1 != 0);
	assert(pPerson2 != 0);

	double Pi = pPerson1->getNumberOfRelationships();
	double Pj = pPerson2->getNumberOfRelationships();
	double tBi = pPerson1->getDateOfBirth();
	double tBj = pPerson2->getDateOfBirth();
	double Dpi = pPerson1->getPreferredAgeDifference();
	double Dpj = pPerson2->getPreferredAgeDifference();
	double Ai = pPerson1->getAgeAt(ageRefYear);
	double Aj = pPerson2->getAgeAt(ageRefYear);

	double A = a0 + a1*Pi + a2*Pj + a3*(Pi-Pj) - a4*(tBi+tBj)/2.0 - b*tr;
	double B = a4 + b;

	double ageDebut = EventDebut::getDebutAge();
	double a5 = agfmConst + agfmExp * std::exp( agfmAge*(Ai-ageDebut) );
	double a9 = agfwConst + agfwExp * std::exp( agfwAge*(Aj-ageDebut) );

	A += a5*std::abs(Ai - Aj- Dpi - a8*Ai);
	A += a9*std::abs(Ai - Aj- Dpj - a10*Aj);

	setAB(A, B);
}

HazardFunctionFormationAgeGapRefYear::~HazardFunctionFormationAgeGapRefYear()
{
}

