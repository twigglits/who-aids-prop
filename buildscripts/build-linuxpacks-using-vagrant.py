#!/usr/bin/env python

import sys
import re
import os
import subprocess
import tempfile
import glob
import shutil
import pprint

def rm(n):
    try:
        os.unlink(n)
    except Exception as e:
        #print("Error deleting '{}': {}".format(n,e))
        pass

def testUsingVagrant(packageName, name, entry, workDir):
    print("Testing '{}'".format(name))

    vagrantFileData = open(entry["file"], "rt").read()
    vagrantFileData += """
Vagrant.configure(2) do |config|
  config.vm.provision "shell", inline: <<-SHELL
    cd /vagrant
    rm -f ERROR
    rm -f SUCCESS
    X=`which apt-get`
    if ! [ -z "$X" ] ; then
        sudo apt-get update -y
        sudo dpkg -i "{}"
        sudo apt-get install -f -y
    else
        X=`which dnf`
        if ! [ -z "$X" ] ; then
            dnf install -y ./{}
        else
            echo "System neither has apt-get not dnf" > ERROR
            exit -1
        fi
    fi

    # test the executable

    if ! simpact-cyan-release --showconfigoptions ; then
        echo "Unable to run simpact executable" > ERROR
        exit -1
    fi

    # test the python script
    source /etc/profile.d/simpactcyan.sh # adjust the python path

    PYTHON=python
    X=`which $PYTHON`
    if [ -z "$X" ] ; then
        PYTHON=python3
    fi

    if ! $PYTHON -c "import pysimpactcyan;simpact = pysimpactcyan.PySimpactCyan();r = simpact.run(None,'/tmp');print(r)" ; then
        echo "Unable to use python interface"
        exit -1
    fi

    touch SUCCESS
  SHELL
end
""".format(packageName, packageName)

    os.chdir(workDir)
    open("Vagrantfile", "wt").write(vagrantFileData)

    # Make sure that the status reporting files are gone, as well
    # as the old ssh config file
    rm("ERROR")
    rm("SUCCESS")
    rm("sshcfg")

    print("  Running vagrant.")
    with open("vagranttestlog-{}".format(name), "wb") as f:

        try:
            subprocess.check_call( [ "vagrant", "up" ], stdout=f, stderr=f)
            print("  Done.")

            if not os.path.exists("ERROR") and not os.path.exists("SUCCESS"):
                # It's possible that the directory was shared using rsync instead of sync,
                # try to copy the files

                # Get the ssh config
                with open("sshcfg","wb") as sshCfg:
                    subprocess.call(["vagrant", "ssh-config"], stdout=sshCfg, stderr=f)

                # And manually copy the files
                subprocess.call(["scp", "-F", "sshcfg", "default:/vagrant/ERROR", "default:/vagrant/SUCCESS", "."], stdout=f, stderr=f)

            if os.path.exists("ERROR"):
                raise Exception("Error during build")
            if not os.path.exists("SUCCESS"):
                raise Exception("No build error found, but success state not indicated")

            return True

        finally:
            subprocess.call( [ "vagrant", "destroy", "-f" ], stdout=f, stderr=f)

def buildUsingVagrant(fullFileName, name, entry, workDir):
    print("Building '{}'".format(name))

    if os.path.exists(fullFileName):
        fileName = os.path.basename(fullFileName)
        destFile = os.path.join(workDir, fileName)
        if not os.path.exists(destFile): # Don't copy it if not needed
            shutil.copy(fullFileName, destFile)
    else: # Assume it's an url
        fileName = fullFileName

    vagrantFileData = open(entry["file"], "rt").read()
    vagrantFileData += """
Vagrant.configure(2) do |config|
  config.vm.provision "shell", inline: <<-SHELL
    cd /vagrant
    rm -f ERROR
    rm -f SUCCESS
    X=`which python`
    if ! [ -z "$X" ] ; then
        CMD="python build.py {} --copypackage"
    else
        X=`which python3`
        if ! [ -z "$X" ] ; then
            CMD="python3 build.py {} --copypackage"
        else
            echo "Unable to find a python interpreter to run the build script" >> ERROR
            cat ERROR
            exit -1
        fi
    fi

    if ! $CMD ; then
        echo "Unable to build package" >> ERROR
    else
        touch SUCCESS
    fi
  SHELL
end
""".format(fileName, fileName)

    shutil.copy(entry["script"], os.path.join(workDir, "build.py"))
    shutil.copy("common.py", os.path.join(workDir, "common.py"))

    os.chdir(workDir)
    open("Vagrantfile", "wt").write(vagrantFileData)

    def getPackageFiles(useScp = False):
        if not useScp:
            return set(glob.glob("*.rpm") + glob.glob("*.deb"))

        out = subprocess.check_output(["ssh", "-F", "sshcfg", "default", "cd /vagrant ; echo *.deb ; echo *.rpm"])
        out = set(out.split())
        out.discard("*.deb")
        out.discard("*.rpm")
        return out
 
    # Make sure that the status reporting files are gone, as well
    # as the old ssh config file
    rm("ERROR")
    rm("SUCCESS")
    rm("sshcfg")

    print("  Running vagrant.")
    with open("vagrantbuildlog-{}".format(name), "wb") as f:

        try:
            beforeFiles = getPackageFiles()
            subprocess.check_call( [ "vagrant", "up" ], stdout=f, stderr=f)
            print("  Done.")

            useScp = False
            if not os.path.exists("ERROR") and not os.path.exists("SUCCESS"):
                # It's possible that the directory was shared using rsync instead of sync,
                # try to copy the files

                # Get the ssh config
                with open("sshcfg","wb") as sshCfg:
                    subprocess.call(["vagrant", "ssh-config"], stdout=sshCfg, stderr=f)

                # And manually copy the files
                subprocess.call(["scp", "-F", "sshcfg", "default:/vagrant/ERROR", "default:/vagrant/SUCCESS", "."], stdout=f, stderr=f)
                useScp = True

            if os.path.exists("ERROR"):
                raise Exception("Error during build")
            if not os.path.exists("SUCCESS"):
                raise Exception("No build error found, but success state not indicated")

            afterFiles = getPackageFiles(useScp)
            newFiles = afterFiles.difference(beforeFiles)

            #print(beforeFiles)
            #print(afterFiles)
            #print(newFiles)

            if len(newFiles) == 0:
                raise Exception("Vagrant build appeard to be successful, but no new files were found")
            if len(newFiles) > 1:
                raise Exception("Vagrant build appeard to be successful, more than one new file was found")

            outFile = [x for x in newFiles][0] # The new file

            if useScp and not os.path.exists(outFile):
                subprocess.check_call(["scp", "-F", "sshcfg", "default:/vagrant/" + outFile, "."], stdout=f, stderr=f)

            if not os.path.exists(outFile):
                raise Exception("Couldn't find the reported output file '{}'".format(outFile))

            return outFile
        finally:
            subprocess.call( [ "vagrant", "destroy", "-f" ], stdout=f, stderr=f)

cfgs = {   
    "12.04_32": { "file": "vagrantfiles/12.04_32/Vagrantfile", "script": "build-debian.py" },
    "12.04_64": { "file": "vagrantfiles/12.04_64/Vagrantfile", "script": "build-debian.py" },
    "14.04_32": { "file": "vagrantfiles/14.04_32/Vagrantfile", "script": "build-debian.py" },
    "14.04_64": { "file": "vagrantfiles/14.04_64/Vagrantfile", "script": "build-debian.py" },
    "16.04_32": { "file": "vagrantfiles/16.04_32/Vagrantfile", "script": "build-debian.py" },
    "16.04_64": { "file": "vagrantfiles/16.04_64/Vagrantfile", "script": "build-debian.py" },
    "fc23_64": { "file": "vagrantfiles/fc23_64/Vagrantfile", "script": "build-fedora.py" },
    "fc24_64": { "file": "vagrantfiles/fc24_64/Vagrantfile", "script": "build-fedora.py" },
    "debian8_64": { "file": "vagrantfiles/debian8_64/Vagrantfile", "script": "build-debian.py" },
}

def main():

    startDir = os.getcwd()



    def printConfigNames(cfgs):
        names = [ n for n in cfgs ]
        names.sort()
        for n in names:
            print("  {}".format(n))
        print("")
        return names

    try:
        if len(sys.argv) < 3:
            raise Exception("At least a source path and one configuration needs to be specified. Nothing to do now.")
        
        fullFileName = sys.argv[1]

        buildNames = set()
        for o in sys.argv[2:]:
            if o in cfgs: # Add it if it matches exactly
                buildNames.add(o)
            else: # Try to interpret it as a regular expression
                for cfgName in cfgs:
                    match = re.match(o, cfgName)
                    if not match or match.group(0) != cfgName:
                        pass
                    else:
                        buildNames.add(cfgName)

        if len(buildNames) == 0:
            raise Exception("No matching build configurations were detected")

    except Exception as e:
        print("""
Error: {}

Usage: {} file.tar.gz name1 [ name2 ... ]

Each name can be an exact configuration name, or can be a regular 
expression that matches one or more of these names.
""".format(e, sys.argv[0]))
        print("Valid configuration names are: ")
        printConfigNames(cfgs)
        sys.exit(-1)

    try:
        print("Checking if 'vagrant' exists")
        subprocess.check_call(["vagrant", "-v"])
    except subprocess.CalledProcessError:
        pass
    except Exception as e:
        print("""Error: {}

Couldn't run 'vagrant', which is needed to create the package files.
See https://www.vagrantup.com/
""".format(e))
        sys.exit(-1)
    
    workDir = tempfile.mkdtemp(prefix = "simpactbuildtmp")
    print("Work directory is " + workDir)

    print("Building the following configurations:")
    buildNames = printConfigNames(buildNames)

    results = [ ]
    for n in buildNames:
        entry = cfgs[n]

        cfgInfo = { "name": n,
                    "package": None,
                    "builderror": None,
                    "testsuccess": False,
                    "testerror": None }

        packageName = None
        try:
            os.chdir(startDir) # Make sure we always cleanly start in this directory

            packageName = buildUsingVagrant(fullFileName, n, entry, workDir)
            cfgInfo["package"] = packageName
            print("  Build ok: {}".format(packageName))
        except Exception as e:
            cfgInfo["builderror"] = str(e)
            print("  Build error: {}".format(e))

        if packageName:
            try:
                os.chdir(startDir) # Make sure we always cleanly start in this directory

                testUsingVagrant(packageName, n, entry, workDir)
                cfgInfo["testsuccess"] = True
                print("  Package test ok")
            except Exception as e:
                cfgInfo["testerror"] = str(e)
                print("  Package test error: {}".format(e))

        results.append(cfgInfo)

    #pprint.pprint(results)
    print("")
    print("Summary:")
    print("--------")
    for r in results:
        buildMsg = "undefined"
        testMsg = ""
        if r["package"]:
            buildMsg = r["package"]
            if r["testsuccess"]:
                testMsg = "OK"
            else:
                if r["testerror"]:
                    testMsg = r["testerror"]
                else:
                    testMsg = "unknown test error"

        else:
            if r["builderror"]:
                buildMsg = r["builderror"]
            else:
                buildMsg = "unknown build error"

        print("{}\t{}\t{}".format(r["name"], buildMsg, testMsg))

    print("")
    print("Work directory is " + workDir)

def testMain():
    workDir = "/tmp/simpactbuildtmp5m9fIl"
    n = "fc24_64"
    packageName = "simpactcyan-0.19.6-1.fc24.x86_64.rpm"

    testUsingVagrant(packageName, n, cfgs[n], workDir)

if __name__ == "__main__":
    #testMain()
    main()

