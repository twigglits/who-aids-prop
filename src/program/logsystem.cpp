#include "logsystem.h"
#include "configsettings.h"
#include "configwriter.h"

using namespace std;

void LogSystem::processConfig(ConfigSettings &config)
{
	string eventLogFile, personLogFile, relationLogFile, treatmentLogFile;

	if (!config.getKeyValue("logsystem.outfile.logevents", eventLogFile) ||
	    !config.getKeyValue("logsystem.outfile.logrelations", relationLogFile) ||
	    !config.getKeyValue("logsystem.outfile.logpersons", personLogFile) ||
	    !config.getKeyValue("logsystem.outfile.logtreatments", treatmentLogFile)
	    )
		abortWithMessage(config.getErrorString());

	if (eventLogFile.length() > 0)
	{
		if (!logEvents.open(eventLogFile))
			abortWithMessage("Unable to open event log file: " + logEvents.getErrorString());
	}

	if (personLogFile.length() > 0)
	{
		if (!logPersons.open(personLogFile))
			abortWithMessage("Unable to open person log file: " + logPersons.getErrorString());
	}

	if (relationLogFile.length() > 0)
	{
		if (!logRelations.open(relationLogFile))
			abortWithMessage("Unable to open relationship log file: " + logRelations.getErrorString());
	}

	if (treatmentLogFile.length() > 0)
	{
		if (!logTreatment.open(treatmentLogFile))
			abortWithMessage("Unable to open treatment log file: " + logTreatment.getErrorString());
	}

	logPersons.print("\"ID\",\"Gender\",\"TOB\",\"TOD\",\"IDF\",\"IDM\",\"TODebut\",\"FormEag\",\"InfectTime\",\"InfectOrigID\",\"InfectType\",\"log10SPVL\",\"TreatTime\"");
	logRelations.print("\"IDm\",\"IDw\",\"FormTime\",\"DisTime\",\"AgeGap\"");
	logTreatment.print("\"ID\",\"Gender\",\"TStart\",\"TEnd\",\"DiedNow\"");
}

void LogSystem::obtainConfig(ConfigWriter &config)
{
	if (!config.addKey("logsystem.outfile.logevents", logEvents.getFileName()) ||
	    !config.addKey("logsystem.outfile.logrelations", logRelations.getFileName()) ||
	    !config.addKey("logsystem.outfile.logpersons", logPersons.getFileName()) ||
	    !config.addKey("logsystem.outfile.logtreatments", logTreatment.getFileName())
	    )
		abortWithMessage(config.getErrorString());
}

LogFile LogSystem::logEvents;
LogFile LogSystem::logPersons;
LogFile LogSystem::logRelations;
LogFile LogSystem::logTreatment;
