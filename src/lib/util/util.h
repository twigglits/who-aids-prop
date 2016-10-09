#ifndef UTIL_H

#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

//long double GetSystemTime();
bool ReadInputLine(FILE *fi, std::string &line);
void SplitLine(const std::string &line, std::vector<std::string> &args, const std::string &separatorChars = " \t",
	       const std::string &quoteChars = "\"'", const std::string &commentStartChars = "#", 
	       bool ignoreZeroLengthFields = true);

std::string createFullPath(const std::string &dir, const std::string &file);

void abortWithMessage(const std::string &msg);

bool parseAsInt(const std::string &str, int &number);
bool parseAsInt(const std::string &str, int64_t &number);
bool parseAsDouble(const std::string &str, double &number);
bool parseAsDoubleVector(const std::string &str, std::vector<double> &numbers, std::string &badField);
std::string doubleToString(double x);

std::string trim(const std::string &str, const std::string &trimChars = " \t\r\n");
std::string replace(const std::string &input, const std::string &target, const std::string &replacement);

#endif // UTIL_H

