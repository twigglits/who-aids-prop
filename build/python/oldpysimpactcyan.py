""" Python bindings for running programs from the SimpactCyan package.

This package supplies a class `PySimpactCyan`, which can be used to run
simulations using the SimpactCyan programs. When used as a standalone
python program, it can be used to list a full config file based on just$
a few fields which override the defaults.
"""

from __future__ import print_function
import sys
import pprint
import copy
import subprocess
import json
import os
import tempfile
import shutil
import random
import time
import platform

# This helper function executes 'executable', for example
#
#     [ 'simpact-cyan-opt', '--showconfigoptions' ]
#
# to retrieve all config options in JSON format. It then expands the
# options which are of type 'distTypes' into the available 1D distribution
# types listed in the JSON config

def _getExpandedSettingsOptions(executable):

    with open(os.devnull, "w") as nullFile:
        proc = subprocess.Popen(executable, stdout=subprocess.PIPE, stderr=nullFile)
        jsonData, unusedErr = proc.communicate()
        proc.wait()

    jsonData = jsonData.decode("utf-8") # Needed for python3

    configOptions = json.loads(jsonData)
    configNames = configOptions["configNames"]
    
    distTypesNames = [ ]
    for n in configOptions:
        if n != "configNames":
            distTypesNames.append(n)

    possibleDistNames = { }
    for n in distTypesNames:
        possibleDistNames[n] = [ t for t in configOptions[n] ]

    # Change the config entries which have a 'distTypes' etc setting

    newConfig = copy.deepcopy(configNames)
    for n in configNames:

        params = configNames[n]['params']
        for i in range(len(params)):
            p = params[i] 
            pName = p[0]
            pValue = p[1]

            if pValue in distTypesNames:

                distTypes = configOptions[pValue]

                defaultDistName = "fixed"
                defaultDistParams = None
                if len(p) == 3: # The third parameter are the defaults
                    defaultDistOptions = p[2]
                    defaultDistName = defaultDistOptions[0]
                    defaultDistParams = defaultDistOptions[1]

                # Adjust the entry in 'newConfig' to reflect the default distribution name
                newConfig[n]['params'][i] = [ pName + ".type", defaultDistName, possibleDistNames[pValue] ]

                # Add entries to newConfig for all possible distributions
                for distName in distTypes:
                    distParams = distTypes[distName]['params']

                    newConfName = n + "_" + str(i) + "_" + distName
                    newConfig[newConfName] = { 'depends': [ n, pName + ".type", distName ] }

                    if defaultDistName == distName and defaultDistParams:
                        # Default parameters are present for this distribution,
                        # merge them

                        defaultParamMap = { }
                        for dp,dv in defaultDistParams:
                            defaultParamMap[dp] = dv

                        modParams = copy.deepcopy(distParams)
                        for paramPair in modParams:
                            dp = paramPair[0]
                            if dp in defaultParamMap:
                                paramPair[1] = defaultParamMap[dp]

                        newConfig[newConfName]['params'] = [ [ pName + "." + distName + "." + p[0] ] + p[1:] for p in modParams ]
                    else:
                        # No specific defaults, just use the info from the "distTypes" data structure
                        newConfig[newConfName]['params'] = [ [ pName + "." + distName + "." + p[0] ] + p[1:] for p in distParams ]

                    newConfig[newConfName]['info'] = distTypes[distName]['info']

    #pprint.pprint(newConfig)
    #sys.exit(-1)
    return newConfig

# userConfig is a map of key/value pairs, for each configuration option as
# will be present in the config file
#
# cfg is actually an entry of configNames, and configNames is present as
# well in case this entry has a dependency, which then needs to be checked
# first
#
# requiredKeys will be a map of keys than need to be set in the final config
# file, either mapped to None just to indicate that it needs to be present,
# or mapped to a list of possible values
def _processConfigPart(cfg, userConfig, configNames, requiredKeys):

    params = cfg['params']
    deps = cfg['depends']

    # Check if we need to process dependencies first
    if deps is not None:

        depObjName = deps[0]
        depObj = configNames[depObjName]
        depKey = deps[1]
        depVal = deps[2]
        
        #print "processConfigPart", depObjName
        #pprint.pprint(depObj)
        if not _processConfigPart(depObj, userConfig, configNames, requiredKeys):
            # Parent dependency not fulfilled, so this one isn't either
            return False
        #print "done: processConfigPart", depObjName

        if not depKey in userConfig:
            pprint.pprint(userConfig)
            raise Exception("Key %s was not set" % depKey)
        
        if userConfig[depKey] != depVal:
            return False # Dependency not fulfilled

        for k in params:
            if len(k) == 3:
                requiredKeys[k[0]] = k[2]
            else:
                requiredKeys[k[0]] = None

    for p in params:

        key = p[0]
        val = p[1]

        if len(p) == 3:

            requiredKeys[key] = p[2]
        else:
            requiredKeys[key] = None

        # See if we should check defaults
        if not key in userConfig: 
            #if val is None:
            #    raise Exception("Key %s is not set" % key)

            userConfig[key] = val
        
    return True

def createConfigLines(executable, inputConfig, checkNone = True, ignoreKeys = [ ]):

    def getValidNames(strList):
        r = "['" + strList[0] + "'"
        for i in range(1,len(strList)):
            r += ", '" + strList[i] + "'"
        r += "]"
        return r

    userConfig = copy.deepcopy(inputConfig)
    configNames = _getExpandedSettingsOptions(executable)

    requiredKeys = { }

    for n in configNames:
        cfg = configNames[n]
        _processConfigPart(cfg, userConfig, configNames, requiredKeys)

    for k in userConfig:
        if not k in requiredKeys:
            raise Exception("Encountered unknown key %s" % k)

        val = userConfig[k]

        possibleValues = requiredKeys[k]
        if possibleValues is not None:
            if not val in possibleValues:
                raise Exception("Value '%s' for key %s is not allowed, should be one of %s" % (val, k, possibleValues))

        if checkNone:
            if val is None:
                raise Exception("Key %s is not set" % k)

    # Display the final config file

    lines = [ ]
    unusedlines = [ ]
    # In principle this should contain the same info as userConfig at the end,
    # but we'll introduce some ordering here so we can feed it back to R in a better
    # way
    resultingConfig = [ ] 

    names = [ key for key in configNames ]
    names.sort()
    for key in names:
        deps = configNames[key]["depends"]
        params = configNames[key]["params"]
        info = configNames[key]["info"]

        if info:
            info = "\n".join(info)

        usedparams = [ ]
        unusedparams = [ ]
        for p in params:
            k = p[0]
            if k in requiredKeys:
                
                v = userConfig[k]
                ns = 60-len(k)
                k += " "*ns

                if len(p) == 3: # Limited number of possibilities
                    usedparams.append("# Valid values are: " + getValidNames(p[2]))

                if v is None:
                    usedparams.append("%s = " % k)
                elif type(v) == float:
                    usedparams.append("%s = %.15g" % (k, v))
                elif type(v) == int:
                    usedparams.append("%s = %d" % (k, v))
                else:
                    usedparams.append("%s = %s" % (k, str(v)))

                idx = len(resultingConfig)+1

                if v is None:
                    resultingConfig.append((idx, p[0], ""))
                else:
                    resultingConfig.append((idx, p[0], v))

            else:
                unusedparams.append("# " + p[0])

        if usedparams:
            if deps:
                lines += [ "# The following depends on %s = %s" % (deps[1], deps[2]) ]

            if info:
                lines += [ "# " + l for l in info.splitlines() ]

            lines += usedparams
            lines += [ "" ]

        if unusedparams:
            if deps:
                unusedlines += [ "# The following depends on %s = %s" % (deps[1], deps[2]) ]

            unusedlines += unusedparams
            unusedlines += [ "#" ]

    introlines = [  "# The configuration file format is quite straightforward, it is just a set of",
                    "# 'key = value' lines. Lines that start with '#' are treated as comments and",
                    "# are ignored.",
                    "#",
                    "# If the key starts with a dollar ('$') sign, the rest of the key is ",
                    "# considered to be the name of a variable, which may be used later on in the",
                    "# file. To use such a variable in a specified value, you need to surround",
                    "# the variable name with '${' and '}'. For example, one could write:",
                    "#",
                    "#     $PREFIX = MyCustomPrefix",
                    "#     logsystem.outfile.logevents = ${PREFIX}-output.log",
                    "#",
                    "# and the file used will have the name 'MyCustomPrefix-output.log'.",
                    "#",
                    "# In the same way, environment variables can be used, and, in fact, ",
                    "# environment variables will take precedence over these internal variables.",
                    "# This way, it is easy to change the content of these variables on the command",
                    "# line",
                    "#",
                    "# Note that no calculations can be performed in this file anymore, so instead",
                    "# of writing 1.0/2.0, you'd need to write 0.5 for example.",
                    "" ]

    return (userConfig, introlines + unusedlines + [ "" ] + lines, resultingConfig)

def _replaceVariables(value, variables):

    newValue = ""

    done = False
    prevIdx = 0
    while not done:
        idx = value.find("${", prevIdx)
        if idx < 0:
            done = True
        else:
            nextIdx = value.find("}", idx)
            if nextIdx < 0:
                done = True
            else:
                key = value[idx+2:nextIdx]
                if key in variables:
                    newValue += variables[key]
                    prevIdx = nextIdx+1
                else:
                    newValue += value[prevIdx:nextIdx+1]
                    prevIdx = nextIdx + 1
    
    newValue += value[prevIdx:]

    return newValue.strip()

class PySimpactCyan(object):
    """ This class is used to run SimpactCyan based simulations."""

    def __init__(self):
        self._execPrefix = "simpact-cyan"
        self._dataDirectory = self._findSimpactDataDirectory()
        self._execDir = self._findSimpactDirectory()

    def setSimpactDirectory(self, dirName):
        """ Sets the directory in which the simpact binaries were installed to `dirName`"""
        self._execDir = dirName

    def setSimpactDataDirectory(self, dirName):
        self._dataDirectory = os.path.abspath(dirName)

    def _findSimpactDataDirectory(self):
        paths = [ ]
        if platform.system() == "Windows":
            paths += [ "C:\\Program Files (x86)\\SimpactCyan\\data\\", "C:\\Program Files\\SimpactCyan\\data\\" ]
        else:
            paths += [ "/usr/share/simpact-cyan/", "/usr/local/share/simpact-cyan/" ]
            paths += [ "/Applications/SimpactCyan.app/Contents/data/" ]

        for p in paths:
            f = os.path.join(p, "sa_2003.csv")
            if os.path.exists(f):
                print("Setting data directory to", p)
                return p

        print("Warning: can't seem to find a way to run the simpact data directory")
        return None

    def _findSimpactDirectory(self):
        with open(os.devnull, "w") as DEVNULL:
            paths = [ ]
            if platform.system() == "Windows":
                paths += [ "C:\\Program Files (x86)\\SimpactCyan", "C:\\Program Files\\SimpactCyan" ]

            exe = "simpact-cyan-opt" # This should always exist

            # First see if we can run the executable without a full path
            try:
                subprocess.call([ exe ], stderr=DEVNULL, stdout=DEVNULL)
                return None
            except:
                pass

            # Then try some predefined paths
            for p in paths:
                try:
                    subprocess.call([ os.path.join(p,exe) ], stderr=DEVNULL, stdout=DEVNULL)
                    print("Simpact executables found in %s" % p)
                    return p
                except:
                    pass

        print("Warning: can't seem to find a way to run the simpact executables")
        return None

    def setSimulationPrefix(self, prefix):

        if not prefix:
            raise Exception("No valid simulation prefix specified")

        with open(os.devnull, "w") as DEVNULL:
            try:
                p = self._getExecPath(testPrefix = prefix)
                subprocess.call( [ p ], stderr=DEVNULL, stdout=DEVNULL)
                self._execPrefix = prefix
            except Exception as e:
                raise Exception("Unable to use specified prefix '%s' (can't run '%s')" % (prefix, p))

    def _getExecPath(self, opt = True, release = True, testPrefix = None):

        fullPath = testPrefix if testPrefix else self._execPrefix
        fullPath += "-"
        fullPath += "opt" if opt else "basic"

        if not release:
            fullPath += "-debug"

        if self._execDir is not None:
            fullPath = os.path.join(self._execDir, fullPath)
        return fullPath

    def runDirect(self, configFile, parallel = False, opt = True, release = True, outputFile = None, seed = -1, destDir = None):

        fullPath = self._getExecPath(opt, release)
        parallelStr = "1" if parallel else "0"

        if destDir is None:
            destDir = os.path.abspath(os.path.dirname(configFile))
    
        closeOutput = False
        origDir = os.getcwd()
        try:
            os.chdir(destDir)

            if outputFile is not None:
                if os.path.exists(outputFile):
                    raise Exception("Want to write to output file '%s', but this already exists" % outputFile)

                f = open(outputFile, "wt")
                closeOutput = True
            else:
                f = sys.stdout

            newEnv = copy.deepcopy(os.environ)
            if seed >= 0:
                newEnv["MNRM_DEBUG_SEED"] = str(seed)

            if self._dataDirectory is not None:
                newEnv["SIMPACT_DATA_DIR"] = str(self._dataDirectory)
            
            print("Results will be stored in directory '%s'" % os.getcwd())
            print("Running simpact executable...")
            proc = subprocess.Popen([fullPath, configFile, parallelStr], stdout=f, stderr=f, cwd=os.getcwd(), env=newEnv)
            proc.wait() # Wait for the process to finish
            print("Done.")
            print()

            if proc.returncode != 0:
                raise Exception("Program exited with an error code (%d)" % proc.returncode)
            
        finally:
            if closeOutput:
                f.close()

            if closeOutput: # If not in a file, it was already on screen
                # Also show the output on screen
                with open(outputFile, "rt") as f:
                    line = f.readline()
                    while line:
                        sys.stdout.write(line)
                        line = f.readline()
                    f.close()

            os.chdir(origDir)

    def _createConfigLines(self, inputConfig, checkNone = True, ignoreKeys = []):
        executable = [ self._getExecPath(), "--showconfigoptions" ]
        return createConfigLines(executable, inputConfig, checkNone, ignoreKeys)

    def _checkKnownKeys(self, keyList):
        executable = [ self._getExecPath(), "--showconfigoptions" ]

        configNames = _getExpandedSettingsOptions(executable)

        allKnownKeys = [ ]
        for n in configNames:
            paramList = configNames[n]["params"]
            paramKeys = [ ]
            for p in paramList:
                paramKeys.append(p[0])

            allKnownKeys += paramKeys

        for k in keyList:
            if not k in allKnownKeys:
                raise Exception("Encountered unknown key '%s'" % k)

    def getConfiguration(self, config, show = False):
        # Make sure config is a dict
        if not config:
            config = { }

        ignoreKeys = [ "population.agedistfile" ]

        finalConfig, lines, sortedConfig = self._createConfigLines(config, False, ignoreKeys)
        lines.append('')

        if show:
            sys.stdout.write('\n'.join(lines))

        return sortedConfig

    def showConfiguration(self, config):
        self.getConfiguration(config, True)

    def _getID(self, identifierFormat):

        def getRandomChar():
            chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
            pos = int(random.random()*len(chars)) % len(chars)
            return chars[pos]

        t = time.gmtime()
        pid = os.getpid()
        simType = self._execPrefix

        identifier = ""
        prevPos = 0
        pos = identifierFormat.find("%")
        while pos >= 0:
            if pos < len(identifierFormat)-1 and identifierFormat[pos+1] in [ '%', 'T', 'y', 'm', 'd', 'H', 'M', 'S', 'p', 'r' ]:

                identifier += identifierFormat[prevPos:pos]
                prevPos = pos+2

                n = identifierFormat[pos+1]
                if n == '%':
                    identifier += "%"
                elif n == 'T':
                    identifier += simType
                elif n == 'y':
                    identifier += "%d" % t.tm_year
                elif n == 'm':
                    identifier += "%02d" % t.tm_mon
                elif n == 'd':
                    identifier += "%02d" % t.tm_mday
                elif n == 'H':
                    identifier += "%02d" % t.tm_hour
                elif n == 'M':
                    identifier += "%02d" % t.tm_min
                elif n == 'S':
                    identifier += "%02d" % t.tm_sec
                elif n == 'p':
                    identifier += "%d" % pid
                elif n == 'r':
                    identifier += getRandomChar()
                else:
                    raise Exception("Internal error: unexpected identifier format '%s'" % n)

                pos = identifierFormat.find("%", pos+2)
            else:
                # No need to adjust prevPos
                pos = identifierFormat.find("%", pos+1)

        identifier += identifierFormat[prevPos:]

        return identifier

    def _getFromConfigOrSetDefault(self, config, key, defaultValue):

        try:
            v = config[key]
        except:
            v = defaultValue
            config[key] = defaultValue

        return v

    def run(self, config, destDir, agedist = None, parallel = False, opt = True, release = True, seed = -1, interventionConfig = None, dryRun = False, identifierFormat = "%T-%y-%m-%d-%H-%M-%S_%p_%r%r%r%r%r%r%r%r-"):

        if not destDir:
            raise Exception("A destination directory must be specified")

        # Make sure config is a dict
        if not config:
            config = { }

        idStr = self._getID(identifierFormat)
        if not idStr:
            raise Exception("The specified identifier format leads to an empty identifier")

        distFile = None
        if agedist is not None:
            if isinstance(agedist, dict):
                distAges = agedist["Age"]

                if "Percent.Male" in agedist:
                    distMalePct = agedist["Percent.Male"]
                elif "Percent Male" in agedist:
                    distMalePct = agedist["Percent Male"]
                else:
                    raise Exception("Error in age distribution: Key for male percentage must be 'Percent.Male' or 'Percent Male'")

                if "Percent.Female" in agedist:
                    distFemalePct = agedist["Percent.Female"]
                elif "Percent Female" in agedist:
                    distFemalePct = agedist["Percent Female"]
                else:
                    raise Exception("Error in age distribution: Key for female percentage must be 'Percent.Female' or 'Percent Female'")

                if len(distAges) != len(distMalePct) or len(distAges) != len(distFemalePct):
                    raise Exception("Not all columns of the 'agedist' variable seem to have the same length")

                distFile = "%sagedist.csv" % idStr
                config["population.agedistfile"] = distFile

            else: # Assume we're referring to a file

                config["population.agedistfile"] = str(agedist)


        # Intervention event stuff

        intTimes = None
        intBaseFile = None
        ivIDs = []
        if interventionConfig:
            # Lets make sure we order the intervention times

            intTimes = [ ]
            if type(interventionConfig) == dict:

                for iv in interventionConfig:
                    t = float(interventionConfig[iv]["time"])
                    del interventionConfig[iv]["time"]
                    intTimes.append( (t, interventionConfig[iv]) )

            else: # assume it's a list
                for iv in interventionConfig:
                    t = float(iv["time"])
                    del iv["time"]
                    intTimes.append( (t, iv) )

            intTimes.sort() # Make sure it's sorted on time, interpreted as a real number

        if intTimes:

            config["intervention.enabled"] = "yes"            

            isFirstTime = True
            ivTimeString = ""
            ivIDString = ""
            count = 1
            for (t,iv) in intTimes:
                if not isFirstTime:
                    ivTimeString += ","
                    ivIDString += ","
                
                ivTimeString += str(t)
                ivIDString += str(count)
                ivIDs.append(str(count))

                isFirstTime = False
                count += 1
                
            config["intervention.times"] = ivTimeString
            config["intervention.fileids"] = ivIDString

            intBaseFile = "%sinterventionconfig_%%.txt" % idStr
            config["intervention.baseconfigname"] = intBaseFile

        if os.path.exists(destDir):
            # Check that we're actually dealing with a directory
            if not os.path.isdir(destDir):
                raise Exception("Specified destination directory '%s' exists but does not seem to be a directory" % destDir)

        else:
            # Create the directory
            print("Specified destination directory '%s' does not exist, creating it" % destDir)
            os.makedirs(destDir)

        # Here, the actual configuration file lines are created
        finalConfig, lines, notNeeded = self._createConfigLines(config, True)

        # Check some paths
        configFile = os.path.abspath(os.path.join(destDir, "%sconfig.txt" % idStr))
        if os.path.exists(configFile):
            raise Exception("Want to write to configuration file '%s', but this already exists" % configFile)

        outputFile = os.path.abspath(os.path.join(destDir, "%soutput.txt" % idStr))
        if os.path.exists(outputFile):
            raise Exception("Want to write to output file '%s', but this already exists" % outputFile)

        if distFile:
            fullDistFile = os.path.abspath(os.path.join(destDir, distFile))
            if os.path.exists(fullDistFile):
                raise Exception("Want to write to age distribution file '%s', but this already exists" % fullDistFile)

        # Write the config file
        with open(configFile, "wt") as f:
            f.write("# Some variables. Note that if set, environment variables will have\n")
            f.write("# Precedence.\n")
            f.write("$SIMPACT_OUTPUT_PREFIX = %s\n" % idStr)
            if self._dataDirectory:
                f.write("$SIMPACT_DATA_DIR = %s\n" % self._dataDirectory)	
            f.write("\n")

            for l in lines:
                f.write(l + "\n")
            f.close()

        if distFile:
            # Write the age distribution file
            with open(fullDistFile, "wt") as f:
                f.write("Age,Percent Male,Percent Female\n")
                for i in range(len(distAges)):
                    f.write("%g,%g,%g\n" % (distAges[i], distMalePct[i], distFemalePct[i]))
                f.close()

        # write intervention config files
        if intTimes:
            for tIdx in range(len(intTimes)):

                t = intTimes[tIdx][0]
                iv = intTimes[tIdx][1]

                # With the current approach, the best we can do is to check for keys that
                # are never used in a config file
                self._checkKnownKeys([ name for name in iv ])

                fileName = intBaseFile.replace("%", ivIDs[tIdx])
                fileName = os.path.join(destDir, fileName)
                with open(fileName, "w") as f:
                    for k in iv:
                        f.write("%s = %s\n" % (k,iv[k]))
                    f.close()

        # Set environment variables (if necessary) and start executable

        if not dryRun:
            print("Using identifier '%s'" % idStr)
            self.runDirect(configFile, parallel, opt, release, outputFile, seed, destDir)

        # Create the return structure
        results = { }
        replaceVars = { }
        
        # These are things that should be replaced
        if self._dataDirectory: 
            replaceVars["SIMPACT_DATA_DIR"] = self._dataDirectory
        replaceVars["SIMPACT_OUTPUT_PREFIX"] = idStr
        
        # These are the output log files in a generic way
        outFileSpec = ".outfile."
        for n in finalConfig:

            if outFileSpec in n:
                value = _replaceVariables(finalConfig[n], replaceVars)

                if value:
                    pos = n.find(outFileSpec) + len(outFileSpec)
                    logName = n[pos:]
                    results[logName] = os.path.join(destDir, value)

        results["configfile"] = os.path.join(destDir, configFile)
        results["outputfile"] = os.path.join(destDir, outputFile)
        results["id"] = idStr

        if distFile:
            results["agedistfile"] = os.path.join(destDir, distFile)
        else:
            results["agedistfile"] = _replaceVariables(finalConfig["population.agedistfile"], replaceVars)

        return results

def main():

    try:
        executable = [ sys.argv[1], "--showconfigoptions"]
        if len(sys.argv[1:]) != 1:
            raise Exception("Invalid number of arguments")
    except Exception as e:
        print("Error: %s" % str(e), file=sys.stderr)
        print(file=sys.stderr)
        print("Usage: pysimpactcyan.py simpactexecutable", file=sys.stderr)
        sys.exit(-1)

    # Read the input

    userConfig = { }
    line = sys.stdin.readline()
    while line:

        line = line.strip()
        if line:
            parts = [ p.strip() for p in line.split('=') ]
            
            key, value = parts[0], parts[1]
            userConfig[key] = value

        line = sys.stdin.readline()

    # In principle, the 'resultingConfigNotNeeded' should contain the same things
    # as finalConfig, but some ordering was introduced
    (finalConfig, lines, resultingConfigNotNeeded) = createConfigLines(executable, userConfig, False)

    lines.append('')
    sys.stdout.write('\n'.join(lines))

if __name__ == "__main__":
    main()


