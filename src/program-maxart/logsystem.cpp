#include "logsystem.h"
#include "configsettings.h"
#include "configwriter.h"

using namespace std;

void LogSystem::processConfig(ConfigSettings &config)
{
	string eventLogFile, personLogFile, relationLogFile, treatmentLogFile;

	if (!config.getKeyValue("logsystem.filename.events", eventLogFile) ||
	    !config.getKeyValue("logsystem.filename.relations", relationLogFile) ||
	    !config.getKeyValue("logsystem.filename.persons", personLogFile) ||
	    !config.getKeyValue("logsystem.filename.treatments", treatmentLogFile)
 	    )
		abortWithMessage(config.getErrorString());

	if (!logEvents.open(eventLogFile))
		abortWithMessage("Unable to open event log file: " + logEvents.getErrorString());

	if (!logPersons.open(personLogFile))
		abortWithMessage("Unable to open person log file: " + logPersons.getErrorString());

	if (!logRelations.open(relationLogFile))
		abortWithMessage("Unable to open relationship log file: " + logRelations.getErrorString());

	if (!logTreatment.open(treatmentLogFile))
		abortWithMessage("Unable to open treatment log file: " + logTreatment.getErrorString());

	logPersons.print("\"ID\",\"Gender\",\"TOB\",\"TOD\",\"IDF\",\"IDM\",\"TODebut\",\"FormEag\",\"InfectTime\",\"InfectOrigID\",\"InfectType\",\"log10SPVL\",\"TreatTime\",\"XCoord\",\"YCoord\"");
	logRelations.print("\"IDm\",\"IDw\",\"FormTime\",\"DisTime\",\"AgeGap\"");
	logTreatment.print("\"ID\",\"Gender\",\"TStart\",\"TEnd\",\"DiedNow\"");
}

void LogSystem::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("logsystem.filename.events", logEvents.getFileName()) ||
	    !config.addKey("logsystem.filename.relations", logRelations.getFileName()) ||
	    !config.addKey("logsystem.filename.persons", logPersons.getFileName()) ||
	    !config.addKey("logsystem.filename.treatments", logTreatment.getFileName())
	    )
		abortWithMessage(config.getErrorString());
}

LogFile LogSystem::logEvents;
LogFile LogSystem::logPersons;
LogFile LogSystem::logRelations;
LogFile LogSystem::logTreatment;

