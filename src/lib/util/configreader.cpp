#include "configreader.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

ConfigReader::ConfigReader()
{
}

ConfigReader::~ConfigReader()
{
}

bool ConfigReader::read(const string &fileName)
{
	clear();

	FILE *pFile = fopen(fileName.c_str(), "rt");
	if (pFile == NULL)
	{
		setErrorString("Can't open file '" + fileName + "'");
		return false;
	}
	
	string line;

	while (ReadInputLine(pFile, line))
	{
		if (line.length() > 0 && line[0] != '#') // allow for comments
		{
			size_t s = line.find("=");

			if (s == string::npos)
			{
				setErrorString("Can't find '=' in line: " + line);
				fclose(pFile);
				return false;
			}

			string key = line.substr(0, s);
			string value = line.substr(s+1);

			key = trim(key);

			value = substituteVariables(value);
			value = trim(value);

			if (key.length() == 0)
			{
				setErrorString("Detected an empty key in line: " + line);
				fclose(pFile);
				return false;
			}

			if (key[0] == '$')
			{
				key = key.substr(1);

				if (key.length() == 0)
				{
					setErrorString("Detected empty variable assignment in line: " + line);
					fclose(pFile);
					return false;
				}

				m_variables[key] = value;
			}
			else
			{
				if (m_keyValues.find(key) != m_keyValues.end())
				{
					setErrorString("Key '" + key + "' was found more than once");
					fclose(pFile);
					return false;
				}

				m_keyValues[key] = value;
			}
		}
	}

	fclose(pFile);

	return true;
}

bool ConfigReader::getKeyValue(const string &key, string &value) const
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

void ConfigReader::printAll() const
{
	map<string,string>::const_iterator it = m_keyValues.begin();

	while (it != m_keyValues.end())
	{
		cout << it->first << "=" << it->second << endl;
		it++;
	}
}

void ConfigReader::clear()
{
	m_keyValues.clear();
}

void ConfigReader::getKeys(vector<string> &keys) const
{
	map<string,string>::const_iterator it = m_keyValues.begin();

	keys.clear();
	while (it != m_keyValues.end())
	{
		keys.push_back(it->first);
		it++;
	}
}

// Note: environment variables take precedence over internal variables
//       to make it easier to override the output prefix set by the
//       Python/R bindings
string ConfigReader::substituteVariables(const string &s)
{
	bool done = false;
	size_t startPos = 0;
	string resultStr;
	
	while (!done)
	{
		size_t idx = s.find("${", startPos);

		if (idx == string::npos) // Not found
		{
			done = true;
			resultStr += s.substr(startPos);
		}
		else
		{
			size_t idx2 = s.find("}", idx);

			if (idx2 == string::npos) // End not found
			{
				done = true;
				resultStr += s.substr(startPos);
			}
			else
			{
				resultStr += s.substr(startPos, idx - startPos);

				string varName = s.substr(idx+2, idx2-idx-2);
				string substStr;

				char *pEnvCont = getenv(varName.c_str());
				if (pEnvCont)
					substStr = string(pEnvCont);
				else
				{
					if (m_variables.find(varName) != m_variables.end())
						substStr = m_variables[varName];
				}

				resultStr += substStr;
				startPos = idx2+1;
			}
		}
	}

	return resultStr;
}
