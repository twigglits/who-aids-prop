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
				setErrorString("Can't find '=' in line");
				fclose(pFile);
				return false;
			}

			string key = line.substr(0, s);
			string value = line.substr(s+1);

			key = trim(key);
			value = trim(value);

			if (key.length() == 0)
			{
				setErrorString("Detected an empty key");
				fclose(pFile);
				return false;
			}

			if (m_keyValues.find(key) != m_keyValues.end())
			{
				setErrorString("Key '" + key + "' was found more than once");
				fclose(pFile);
				return false;
			}
			m_keyValues[key] = value;
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

string ConfigReader::trim(const string &s)
{
	int startPos = 0;
	bool done = false;

	while (!done && startPos < s.length())
	{
		char c = s[startPos];

		if (c != '\t' && c != ' ')
			done = true;
		else
			startPos++;
	}

	int endPos = s.size() - 1;
	done = false;

	while (!done && endPos >= startPos)
	{
		char c = s[endPos];

		if (c != '\t' && c != ' ')
			done = true;
		else
			endPos--;
	}

	//cerr << "input: '" << s << "'" << endl;
	//cerr << "startPos: " << startPos << endl;
	//cerr << "endPos: " << endPos << endl;

	if (startPos >= s.size())
		return string("");

	int len = endPos+1 - startPos;
	string trimmed = s.substr(startPos, len);

	//cerr << "trimmed: '" << trimmed << "'" << endl << endl;

	return trimmed;
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

