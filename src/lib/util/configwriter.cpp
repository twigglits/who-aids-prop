#include "configwriter.h"
#include <stdio.h>

#define __STDC_FORMAT_MACROS // Need this for PRId64
#include <inttypes.h>

using namespace std;

ConfigWriter::ConfigWriter()
{
}

ConfigWriter::~ConfigWriter()
{
}

bool ConfigWriter::addKey(const std::string &key, double value)
{
	char str[1024];

	sprintf(str, "%.15g", value);
	return addKey(key, str);
}

bool ConfigWriter::addKey(const std::string &key, int value)
{
	char str[1024];

	sprintf(str, "%d", value);
	return addKey(key, str);
}

bool ConfigWriter::addKey(const std::string &key, int64_t value)
{
	char str[1024];

	sprintf(str, "%" PRId64, value);
	return addKey(key, str);
}

bool ConfigWriter::addKey(const std::string &key, bool value)
{
	if (value)
		return addKey(key, "yes");
	return addKey(key, "no");
}

bool ConfigWriter::addKey(const std::string &key, const char *pStr)
{
	return addKey(key,string(pStr));
}

bool ConfigWriter::addKey(const std::string &key, const std::string &value)
{
	map<string,string>::const_iterator it = m_keyValues.find(key);

	if (it != m_keyValues.end())
	{
		setErrorString("ConfigWriter::addKey: Key '" + key + "' already exists");
		return false;
	}

	m_keyValues[key] = value;
	return true;
}

void ConfigWriter::getKeys(std::vector<std::string> &keys) const
{
	map<string,string>::const_iterator it = m_keyValues.begin();

	keys.clear();
	while (it != m_keyValues.end())
	{
		keys.push_back(it->first);
		it++;
	}
}

bool ConfigWriter::getKeyValue(const std::string &key, std::string &value) const
{
	map<string,string>::const_iterator it;

	it = m_keyValues.find(key);
	if (it == m_keyValues.end())
	{
		setErrorString("Key not found");
		return false;
	}

	value = it->second;

	return true;
}

