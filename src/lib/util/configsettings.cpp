#include "configsettings.h"
#include <stdlib.h>
#include <iostream>

#define __STDC_FORMAT_MACROS // Need this for PRId64
#include <inttypes.h>


using namespace std;

ConfigSettings::ConfigSettings()
{
}

ConfigSettings::~ConfigSettings()
{
}

bool ConfigSettings::load(const std::string &fileName)
{
	ConfigReader reader;

	if (!reader.read(fileName))
	{
		setErrorString("Unable to load file: " + reader.getErrorString());
		return false;
	}

	clear();
	
	vector<string> keys;

	reader.getKeys(keys);
	for (int i = 0 ; i < keys.size() ; i++)
	{
		string value;

		if (!reader.getKeyValue(keys[i], value))
		{
			cerr << "ConfigSettings: internal error: lost value for key " << keys[i] << endl;
			abort();
		}

		m_keyValues[keys[i]] = pair<string,bool>(value, false);
	}

	return true;
}

void ConfigSettings::clear()
{
	m_keyValues.clear();
}

void ConfigSettings::getKeys(std::vector<std::string> &keys) const
{
	map<string,pair<string,bool> >::const_iterator it = m_keyValues.begin();

	keys.clear();
	while (it != m_keyValues.end())
	{
		keys.push_back(it->first);
		it++;
	}
}

bool ConfigSettings::getStringKeyValue(const string &key, string &value, bool &used) const
{
	map<string,pair<string,bool> >::const_iterator it;

	it = m_keyValues.find(key);
	if (it == m_keyValues.end())
	{
		setErrorString("Key '" + key + "' not found");
		return false;
	}

	value = it->second.first;
	used = it->second.second;
	return true;
}

bool ConfigSettings::getKeyValue(const std::string &key, std::string &value, const vector<string> &allowedValues)
{
	map<string,pair<string,bool> >::iterator it;

	it = m_keyValues.find(key);
	if (it == m_keyValues.end())
	{
		setErrorString("Key '" + key + "' not found");
		return false;
	}

	value = it->second.first;

	if (allowedValues.size() > 0)
	{
		bool found = false;

		for (int i = 0 ;!found &&  i < allowedValues.size() ; i++)
		{
			if (allowedValues[i] == value)
				found = true;
		}

		if (!found)
		{
			setErrorString("Specified value for key '" + key + "' is not an allowed value.");
			return false;
		}
	}

	it->second.second = true; // mark the key as used

	return true;
}

void ConfigSettings::getUnusedKeys(std::vector<std::string> &keys) const
{
	map<string,pair<string,bool> >::const_iterator it = m_keyValues.begin();

	keys.clear();
	while (it != m_keyValues.end())
	{
		if (!it->second.second) // boolean flag is still set to false
			keys.push_back(it->first);
		it++;
	}
}

bool ConfigSettings::getKeyValue(const std::string &key, double &value, double minValue, double maxValue)
{
	string valueStr;

	if (!getKeyValue(key, valueStr))
		return false;

	if (!parseAsDouble(valueStr, value))
	{
		setErrorString("Can't interpret value for key '" + key + "' as a floating point number");
		return false;
	}

	if (value < minValue || value > maxValue)
	{
		char str[1024];

		sprintf(str, "The value for '%s' must lie between %g and %g, but is %g", key.c_str(), minValue, maxValue, value);
		setErrorString(str);
		return false;
	}

	return true;
}

bool ConfigSettings::getKeyValue(const std::string &key, int &value, int minValue, int maxValue)
{
	string valueStr;

	if (!getKeyValue(key, valueStr))
		return false;

	if (!parseAsInt(valueStr, value))
	{
		setErrorString("Can't interpret value for key '" + key + "' as an integer number");
		return false;
	}

	if (value < minValue || value > maxValue)
	{
		char str[1024];

		sprintf(str, "The value for '%s' must lie between %d and %d, but is %d", key.c_str(), minValue, maxValue, value);
		setErrorString(str);
		return false;
	}

	return true;
}

bool ConfigSettings::getKeyValue(const std::string &key, int64_t &value, int64_t minValue, int64_t maxValue)
{
	string valueStr;

	if (!getKeyValue(key, valueStr))
		return false;

	if (!parseAsInt(valueStr, value))
	{
		setErrorString("Can't interpret value for key '" + key + "' as an integer number");
		return false;
	}

	if (value < minValue || value > maxValue)
	{
		char str[1024];

		sprintf(str, "The value for '%s' must lie between %" PRId64 " and %" PRId64 ", but is %" PRId64 "", key.c_str(), minValue, maxValue, value);
		setErrorString(str);
		return false;
	}

	return true;
}

void ConfigSettings::clearUsageFlags()
{
	map<string,pair<string,bool> >::iterator it = m_keyValues.begin();

	while (it != m_keyValues.end())
	{
		it->second.second = false;
		it++;
	}
}

void ConfigSettings::merge(const ConfigSettings &src)
{
	map<string,pair<string,bool> >::const_iterator srcIt = src.m_keyValues.begin();
	map<string,pair<string,bool> >::const_iterator srcEndIt = src.m_keyValues.end();

	while (srcIt != srcEndIt)
	{
		string key = srcIt->first;
		pair<string,bool> value = srcIt->second;

		// Overwrites an existing entry or adds a non-existing one
		m_keyValues[key] = value;

		srcIt++;
	}
}

bool ConfigSettings::getKeyValue(const string &key, vector<double> &values, double minValue, double maxValue)
{
	string valueStr;

	if (!getKeyValue(key, valueStr))
		return false;

	string badField;

	if (!parseAsDoubleVector(valueStr, values, badField))
	{
		setErrorString("Can't interpret value for key '" + key + "' as a list of floating point numbers (field '" + badField + "' is bad)");
		return false;
	}

	for (size_t i = 0 ; i < values.size() ; i++)
	{
		double value = values[i];
		if (value < minValue || value > maxValue)
		{
			char str[1024];

			sprintf(str, "Each value for '%s' must lie between %g and %g, but one is %g", key.c_str(), minValue, maxValue, value);
			setErrorString(str);
			return false;
		}
	}

	return true;
}

bool ConfigSettings::getKeyValue(const std::string &key, bool &value)
{
	vector<string> yesNoOptions;
	string yesNo;

	yesNoOptions.push_back("yes");
	yesNoOptions.push_back("no");

	if (!getKeyValue(key, yesNo, yesNoOptions))
		return false;

	if (yesNo == "yes")
		value = true;
	else
		value = false;

	return true;
}

