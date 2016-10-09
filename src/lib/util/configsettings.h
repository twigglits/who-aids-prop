#ifndef CONFIGSETTINGS_H

#define CONFIGSETTINGS_H

/**
 * \file configsettings.h
 */

#include "configreader.h"
#include "util.h"
#include <limits>

/** Helper class to read configuration settings, more advanced than ConfigReader.
 *  
 *  This class works in a similar way as the ConfigReader class and the config
 *  files should have the same format. Extra features of this class are the
 *  ability to interpret specific keys as integer or floating point values for
 *  example, and to check which keys have actually been read.
 */
class ConfigSettings : public errut::ErrorBase
{
public:
	ConfigSettings();
	ConfigSettings(const ConfigSettings &src)						{ m_keyValues = src.m_keyValues; }
	~ConfigSettings();

	ConfigSettings &operator=(const ConfigSettings &src)					{ m_keyValues = src.m_keyValues; return *this; }

	/** Loads the key/value pairs from the config file specified by \c fileName. */
	bool load(const std::string &fileName);

	/** Clears the loaded key/value pairs. */
	void clear();

	/** Returns a list of all the keys in the read config file. */
	void getKeys(std::vector<std::string> &keys) const;

	/** Stores the string value for key parameter \c key into argument \c value, checking
	 *  if the value is one of the allowed values in the list \c allowedValues, if specified. */
	bool getKeyValue(const std::string &key, std::string &value, const std::vector<std::string> &allowedValues = std::vector<std::string>() );

	/** Interprets the value for the specified key as a double precision floating point number,
	 *  checking that it lies withing the bounds if specified. */
	// std::numeric_limits<double>::min() is the smallest in absolute value, can't use that here!
	bool getKeyValue(const std::string &key, double &value, double minValue = -std::numeric_limits<double>::max(), 
	                 double maxValue = std::numeric_limits<double>::max());

	/** Interprets the value for the specified key as an integer number,
	 *  checking that it lies withing the bounds if specified. */
	bool getKeyValue(const std::string &key, int &value, int minValue = std::numeric_limits<int>::min(),
	                 int maxValue = std::numeric_limits<int>::max());

	bool getStringKeyValue(const std::string &key, std::string &value, bool &used) const;
	
	/** Stores a list of keys that have not been read by any of the getKeyValue functions into \c keys (allows
	 *  you to check if the config file contains more lines than necessary). */
	void getUnusedKeys(std::vector<std::string> &keys) const;

	/** Resets the markers that keep track of wether or not a key has been read. */
	void clearUsageFlags();

	/** Merges the specified config settings object into the current one. */
	void merge(const ConfigSettings &src);
private:
	std::map<std::string, std::pair<std::string, bool> > m_keyValues;
};

#endif // CONFIGSETTINGS_H

