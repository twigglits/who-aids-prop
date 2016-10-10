#include "logsystem.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "configfunctions.h"

using namespace std;

void LogSystem::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	string eventLogFile, personLogFile, relationLogFile, treatmentLogFile;
	bool_t r;

	if (!(r = config.getKeyValue("logsystem.outfile.logevents", eventLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logrelations", relationLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logpersons", personLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logtreatments", treatmentLogFile))
	    )
		abortWithMessage(r.getErrorString());

	if (eventLogFile.length() > 0)
	{
		if (!(r = logEvents.open(eventLogFile)))
			abortWithMessage("Unable to open event log file: " + r.getErrorString());
	}

	if (personLogFile.length() > 0)
	{
		if (!(r = logPersons.open(personLogFile)))
			abortWithMessage("Unable to open person log file: " + r.getErrorString());
	}

	if (relationLogFile.length() > 0)
	{
		if (!(r = logRelations.open(relationLogFile)))
			abortWithMessage("Unable to open relationship log file: " + r.getErrorString());
	}

	if (treatmentLogFile.length() > 0)
	{
		if (!(r = logTreatment.open(treatmentLogFile)))
			abortWithMessage("Unable to open treatment log file: " + r.getErrorString());
	}

	logPersons.print("\"ID\",\"Gender\",\"TOB\",\"TOD\",\"IDF\",\"IDM\",\"TODebut\",\"FormEag\",\"InfectTime\",\"InfectOrigID\",\"InfectType\",\"log10SPVL\",\"TreatTime\",\"XCoord\",\"YCoord\",\"AIDSDeath\"");
	logRelations.print("\"IDm\",\"IDw\",\"FormTime\",\"DisTime\",\"AgeGap\"");
	logTreatment.print("\"ID\",\"Gender\",\"TStart\",\"TEnd\",\"DiedNow\"");
}

void LogSystem::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("logsystem.outfile.logevents", logEvents.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logrelations", logRelations.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logpersons", logPersons.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logtreatments", logTreatment.getFileName()))
	    )
		abortWithMessage(r.getErrorString());
}

LogFile LogSystem::logEvents;
LogFile LogSystem::logPersons;
LogFile LogSystem::logRelations;
LogFile LogSystem::logTreatment;

ConfigFunctions logSystemConfigFunctions(LogSystem::processConfig, LogSystem::obtainConfig, "00_LogSystem", "__first__");

JSONConfig logSystemJSONConfig(R"JSON(
        "LogSystem": { 
            "depends": null,
            "params": [ 
                ["logsystem.outfile.logevents", "${SIMPACT_OUTPUT_PREFIX}eventlog.csv" ],
                ["logsystem.outfile.logpersons", "${SIMPACT_OUTPUT_PREFIX}personlog.csv" ],
                ["logsystem.outfile.logrelations", "${SIMPACT_OUTPUT_PREFIX}relationlog.csv" ], 
                ["logsystem.outfile.logtreatments", "${SIMPACT_OUTPUT_PREFIX}treatmentlog.csv" ]
            ],
            "info": null                          
        })JSON");

