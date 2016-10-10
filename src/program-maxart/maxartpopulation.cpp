#include "maxartpopulation.h"
#include "eventstudystart.h"

MaxARTPopulation::MaxARTPopulation(bool parallel, GslRandomNumberGenerator *pRngGen) 
	: SimpactPopulation(parallel, pRngGen)
{
	m_studyStage = PreStudy;
}

MaxARTPopulation::~MaxARTPopulation()
{
}

bool MaxARTPopulation::scheduleInitialEvents()
{
	if (!SimpactPopulation::scheduleInitialEvents())
		return false;

	if (EventStudyStart::isMaxARTStudyEnabled())
	{
		EventStudyStart *pEvt = new EventStudyStart(); // global event
		onNewEvent(pEvt);
	}

	return true;
}
