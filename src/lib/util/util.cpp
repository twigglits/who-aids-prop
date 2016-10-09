#include "util.h"
//#include "matfile.h"
//#include "matclass.h"
#include <string.h>
#include <iostream>
#include <time.h>

/*
long double GetSystemTime()
{
	struct timespec t;

	clock_gettime(CLOCK_REALTIME, &t);

	return (long double)t.tv_sec + (long double)t.tv_nsec/(long double)1000000000;
}*/

#define MAXLEN 4096

bool ReadInputLine(FILE *fi, std::string &line)
{
	if (fi == 0)
		return false;
	
	char str[MAXLEN];
	int len;
	
	if (fgets(str,MAXLEN-1,fi) == 0)
		return false;

	str[MAXLEN-1] = 0;
	len = strlen(str);
	while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n'))
	{
		len--;
		str[len] = 0;
	}
	
	line = std::string(str);
	return true;
}

bool HasCharacter(const std::string &charList, char c)
{
	for (int i = 0 ; i < charList.length() ; i++)
	{
		if (c == charList[i])
			return true;
	}
	return false;
}

void SplitLine(const std::string &line, std::vector<std::string> &args, const std::string &separatorChars,
	       const std::string &quoteChars, const std::string &commentStartChars, bool ignoreZeroLengthFields)
{
	std::vector<std::string> arguments;
	int startPos = 0;

	while (startPos < line.length() && HasCharacter(separatorChars, line[startPos]))
	{
		startPos++;

		if (!ignoreZeroLengthFields)
			arguments.push_back("");
	}

	std::string curString("");
	bool done = false;

	if (startPos >= line.length())
	{
		if (!ignoreZeroLengthFields)
			arguments.push_back("");
	}

	while (startPos < line.length() && !done)
	{
		int endPos = startPos;
		bool endFound = false;
		bool gotSeparator = false;

		while (!endFound && endPos < line.length())
		{
			if (HasCharacter(separatorChars, line[endPos]) || HasCharacter(commentStartChars, line[endPos]))
			{
				curString += line.substr(startPos, endPos-startPos);
				endFound = true;

				if (HasCharacter(separatorChars, line[endPos]))
				{
					gotSeparator = true;
					endPos++;
				}
			}
			else if (HasCharacter(quoteChars, line[endPos]))
			{
				curString += line.substr(startPos, endPos-startPos);

				char quoteStartChar = line[endPos];

				endPos += 1;
				startPos = endPos;

				while (endPos < line.length() && line[endPos] != quoteStartChar)
					endPos++;

				curString += line.substr(startPos, endPos-startPos);

				if (endPos < line.length())
					endPos++;

				startPos = endPos;
			}
			else
				endPos++;
		}

		if (!endFound)
		{
			if (endPos-startPos > 0)
				curString += line.substr(startPos, endPos-startPos);
		}

		if (curString.length() > 0 || !ignoreZeroLengthFields)
			arguments.push_back(curString);

		if (endPos < line.length() && HasCharacter(commentStartChars, line[endPos]))
			done = true;
		else
		{
			startPos = endPos;
			curString = std::string("");


			while (startPos < line.length() && HasCharacter(separatorChars, line[startPos]))
			{
				gotSeparator = true;
				startPos++;

				if (!ignoreZeroLengthFields)
					arguments.push_back("");
			}
			
			if (gotSeparator)
			{
				if (startPos >= line.length())
				{
					if (!ignoreZeroLengthFields)
						arguments.push_back("");
				}
			}
		}
	}

	args = arguments;
}

#if 0
bool Load2DMatlabArray(const std::string &fileName, std::vector<double> &destArray, int &numX, int &numY, std::string &errStr)
{
	MatFile f;

	if (!f.read(fileName))
	{
		errStr = "Can't read from " + fileName + ": " + f.getErrorString();
		return false;
	}

	const std::vector<MatFileEntry *> &entries = f.getEntries();

	if (entries.size() != 1)
	{
		char str[1024];

		sprintf(str, "Can handle only one entry, but got %d", (int)entries.size());
		errStr = std::string(str);
		return false;
	}

	std::string errStr2;

	MatClassBase *pParsedEntry = MatClassBase::interpret(entries[0], errStr2);
	if (pParsedEntry == 0)
	{
		errStr = "Unable to interpret: " + errStr2;
		return false;
	}
	else
	{
		if (pParsedEntry->getType() != MATCLASS_TYPE_DOUBLEARRAY)
		{
			errStr = "Cannot process entry type";
			delete pParsedEntry;
			return false;
		}

		MatClassDoubleArray *pArr = (MatClassDoubleArray *)pParsedEntry;

		size_t dims = pArr->getNumberOfDimensions();
		if (dims != 2)
		{
			char str[1024];

			sprintf(str, "Can only process two dimensional data, but number of dimensions is %d", (int)dims);
			delete pParsedEntry;
			return false;
		}

		int numRows = pArr->getNumberOfEntries(0);
		int numCols = pArr->getNumberOfEntries(1);

		numX = numCols;
		numY = numRows;
	
		destArray.resize(numRows*numCols);

		for (int i = 0 ; i < numRows ; i++)
		{
			for (int j = 0 ; j < numCols ; j++)
			{
				double v =  pArr->getData(i+j*numRows);

				destArray[j+i*numCols] = v;
			}
		}

		delete pParsedEntry;
	}

	return true;
}
#endif

std::string createFullPath(const std::string &dir, const std::string &file)
{
	std::string full = dir;
	if (full.length() > 0 && full[full.length()-1] != '/')
		full += "/";
	full += file;
	return full;
}


