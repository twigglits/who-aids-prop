#include "person_vmmc.h"
#include "vspmodellogweibullwithnoise.h"
#include "vspmodellogdist.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "eventhivtransmission.h"
#include "configfunctions.h"
#include "jsonconfig.h"
#include "logsystem.h"
#include <vector>
#include <cmath>

using namespace std;

PERSON_VMMC::PERSON_VMMC(Person *pSelf) : m_pSelf(pSelf)
{
	assert(pSelf);

	assert(m_pARTAcceptDistribution);
	m_artAcceptanceThreshold = m_pARTAcceptDistribution->pickNumber();

	m_aidsDeath = false;

	assert(m_pLogSurvTimeOffsetDistribution);
	m_log10SurvTimeOffset = m_pLogSurvTimeOffsetDistribution->pickNumber();
}

PERSON_VMMC::~PERSON_VMMC()
{
}

double PERSON_VMMC::pickInheritedSetPointViralLoad(const Person *pOrigin)
{
	assert(m_pVspModel != 0);
	double Vsp0 = pOrigin->hiv().getSetPointViralLoad();

	return m_pVspModel->inheritSetPointViralLoad(Vsp0);
}

ProbabilityDistribution *PERSON_VMMC::m_pARTAcceptDistribution = 0;
addDistributionToConfig(m_pARTAcceptDistribution, config, "person.art.accept.threshold");