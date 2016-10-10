#ifndef MAXARTPOPULATION_H

#define MAXARTPOPULATION_H

#include "simpactpopulation.h"
#include <assert.h>

class MaxARTPopulation : public SimpactPopulation
{
public:
	enum StudyStage { PreStudy, InStudy, PostStudy };

	MaxARTPopulation(bool parallel, GslRandomNumberGenerator *pRngGen);
	~MaxARTPopulation();

	StudyStage getStudyStage() const										{ return m_studyStage; }
	void setInStudy()														{ assert(m_studyStage == PreStudy); m_studyStage = InStudy; }
	void setStudyEnded()													{ assert(m_studyStage == InStudy) ; m_studyStage = PostStudy; }
protected:
	bool scheduleInitialEvents();

	StudyStage m_studyStage;
};

inline MaxARTPopulation &MAXARTPOPULATION(State *pState)
{
	assert(pState != 0);
	return static_cast<MaxARTPopulation &>(*pState);
}

inline const MaxARTPopulation &MAXARTPOPULATION(const State *pState)
{
	assert(pState != 0);
	return static_cast<const MaxARTPopulation &>(*pState);
}

#endif // MAXARTPOPULATION_H
