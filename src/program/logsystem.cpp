#include "logsystem.h"
#include "configsettings.h"
#include "configwriter.h"

using namespace std;

void LogSystem::processConfig(ConfigSettings &config)
{
	string eventLogFile, personLogFile, relationLogFile;

	if (!config.getKeyValue("logsystem.filename.events", eventLogFile) ||
	    !config.getKeyValue("logsystem.filename.relations", relationLogFile) ||
	    !config.getKeyValue("logsystem.filename.persons", personLogFile) )
		abortWithMessage(config.getErrorString());

	if (!logEvents.open(eventLogFile) || !logPersons.open(personLogFile) || !logRelations.open(relationLogFile))
		abortWithMessage("Unable to open a log file: " + logEvents.getErrorString() + "," + logPersons.getErrorString() + "," + logRelations.getErrorString());

	logPersons.print("\"ID\",\"Gender\",\"TOB\",\"TOD\",\"IDF\",\"IDM\",\"TODebut\",\"FormEag\",\"InfectTime\",\"InfectOrigID\",\"InfectType\",\"log10SPVL\",\"TreatTime\"");
	logRelations.print("\"IDm\",\"IDw\",\"FormTime\",\"DisTime\",\"AgeGap\"");
}

void LogSystem::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("logsystem.filename.events", logEvents.getFileName()) ||
	    !config.addKey("logsystem.filename.relations", logRelations.getFileName()) ||
	    !config.addKey("logsystem.filename.persons", logPersons.getFileName()) )
		abortWithMessage(config.getErrorString());
}

LogFile LogSystem::logEvents;
LogFile LogSystem::logPersons;
LogFile LogSystem::logRelations;
