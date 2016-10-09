#include "logfile.h"
#include "util.h"
#include <stdarg.h>
#include <string.h>

using namespace std;

LogFile::LogFile()
{
	m_pFile = 0;
}

LogFile::~LogFile()
{
	close();
}

bool LogFile::open(const std::string &fileName)
{
	if (m_pFile)
	{
		setErrorString("A log file with name '" + fileName + "' has already been opened");
		return false;
	}

	// Check if the file already exists
	FILE *pFile = fopen(fileName.c_str(), "rt");
	if (pFile != 0)
	{
		fclose(pFile);
		setErrorString("Specified log file " + fileName + " already exists");
		return false;
	}

	pFile = fopen(fileName.c_str(), "wt");
	if (pFile == 0)
	{
		setErrorString("Unable to open " + fileName + " for writing");
		return false;
	}

	m_pFile = pFile;
	m_fileName = fileName;
	return true;
} 

void LogFile::close()
{
	if (m_pFile == 0)
		return;
	fclose(m_pFile);
	m_pFile = 0;
	m_fileName = "";
}

void LogFile::print(const char *format, ...)
{
	if (m_pFile == 0)
		return;

	va_list ap;

	va_start(ap, format);
	vfprintf(m_pFile, format, ap);
	va_end(ap);
	
	fwrite("\n", 1, 1, m_pFile);
}

void LogFile::printNoNewLine(const char *format, ...)
{
	if (m_pFile == 0)
		return;

	va_list ap;

	va_start(ap, format);
	vfprintf(m_pFile, format, ap);
	va_end(ap);
}

