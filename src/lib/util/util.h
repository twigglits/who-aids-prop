#ifndef UTIL_H

#define UTIL_H

#include <stdio.h>
#include <string>
#include <vector>

//long double GetSystemTime();
bool ReadInputLine(FILE *fi, std::string &line);
void SplitLine(const std::string &line, std::vector<std::string> &args, const std::string &separatorChars = " \t",
	       const std::string &quoteChars = "\"'", const std::string &commentStartChars = "#", 
	       bool ignoreZeroLengthFields = true);

//bool Load2DMatlabArray(const std::string &fileName, std::vector<double> &destArray, int &numX, int &numY, std::string &errStr);
std::string createFullPath(const std::string &dir, const std::string &file);

#endif // UTIL_H

