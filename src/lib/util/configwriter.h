#ifndef CONFIGWRITER_H

#define CONFIGWRITER_H

#include "errut/errorbase.h"
#include <stdint.h>
#include <vector>
#include <map>

// TODO: merge this with configreader?
class ConfigWriter : public errut::ErrorBase
{
public:
	ConfigWriter();
	~ConfigWriter();

	bool addKey(const std::string &key, double value);
	bool addKey(const std::string &key, int value);
	bool addKey(const std::string &key, int64_t value);
	bool addKey(const std::string &key, bool value);
	bool addKey(const std::string &key, const char *pStr);
	bool addKey(const std::string &key, const std::string &value);

	void getKeys(std::vector<std::string> &keys) const;
	bool getKeyValue(const std::string &key, std::string &value) const;
private:
	std::map<std::string,std::string> m_keyValues;
};

#endif // CONFIGWRITER_H
