#!/usr/bin/env python

import csv
import sys
import pprint
import os

class ConfigToolException(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg

def processRow(row, defaultLines, header, idInFirstCol, prefix, counter):
    
    scenarioID = "%d" % counter
    startCol = 0
    if idInFirstCol:
        startCol = 1
        scenarioID = row[0]

    outputName = prefix + scenarioID + ".txt"

    if os.path.exists(outputName):
        raise ConfigToolException("Output file '%s' already exists!" % outputName)

    fieldMap = { }
    for c in range(startCol, len(header)):
        fieldName = header[c].strip()
        value = row[c].strip()

        if value != '*': # '*' means keep the default value
            value = value.replace('%ID', scenarioID)
            fieldMap[fieldName] = value

    newLines = [ ]
    for line in defaultLines:
        
        keep = True
        key = None

        if line[0] != '#': # comments are just transferred
            if line.strip():
                idx = line.find('=')
                if idx < 0:
                    raise ConfigToolException("Could not find '=' in line '%s' from default config file" % line.strip())

                key = line[:idx].strip()
                if key in fieldMap:
                    keep = False

        if keep:
            newLines.append(line)
        else:
            newLines.append("# MODIFIED: " + line)

    with open(outputName, "wt") as f:
        f.write('''#
# DEFAULTS
#
''')
        for l in newLines:
            f.write(l)

        f.write('''#
# MODIFIED VALUES FROM INPUT SPEC
#
''')
        for k in fieldMap:
            f.write("%s = %s\n" % (k, fieldMap[k]))
    
        f.close()

        print "Wrote to file", outputName

def main():
    try:
        if len(sys.argv) != 4:
            raise ConfigToolException("Invalid number of arguments")

        baseConfigFile = sys.argv[1]
        inputSpec = sys.argv[2]
        outputPrefix = sys.argv[3]
    except Exception as e:
        print >>sys.stderr, "Error:", e
        print >>sys.stderr
        print >>sys.stderr, "Usage:", sys.argv[0], "defaultconfig.txt inputspec.csv outputprefix"
        print >>sys.stderr
        print >>sys.stderr, "Use 'Scenario ID' as the first column in the CSV file to append to the outputprefix"
        print >>sys.stderr, "or omit it to use an automatically generated index. Enter '*' in a cell to keep the"
        print >>sys.stderr, "default value of the config file for this scenario."
        sys.exit(-1)

    try:
        with open(baseConfigFile, "rt") as f:
            baseConfigLines = f.readlines()

        with open(inputSpec, "rt") as f:
            csvReader = csv.reader(f)
            headerFields = csvReader.next()

            if len(headerFields) < 1:
                raise ConfigToolException("Input specification in CSV file should contain at least one column")

            firstColIsID = True if headerFields[0].lower() == "scenario id" else False
            counter = 1
            for row in csvReader:
                processRow(row, baseConfigLines, headerFields, firstColIsID, outputPrefix, counter)
                counter += 1

    except Exception as e:
        print >>sys.stderr, "Error:", e
        print >>sys.stderr


if __name__ == "__main__":
    main()
