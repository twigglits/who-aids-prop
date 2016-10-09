#ifndef LOGSYSTEM_H

#define LOGSYSTEM_H

#include "logfile.h"

class ConfigSettings;
class ConfigWriter;

class LogSystem
{
public:
	static void processConfig(ConfigSettings &config);
	static void obtainConfig(ConfigWriter &config);
	static LogFile logEvents, logPersons, logRelations;
};

#define LogEvent LogSystem::logEvents
#define LogPerson LogSystem::logPersons
#define LogRelation LogSystem::logRelations

#endif // LOGSYSTEM_H
