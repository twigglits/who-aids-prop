#!/usr/bin/env python

from __future__ import print_function
try: # try python2
    from urllib2 import urlopen
except: # python3
    from urllib.request import urlopen
import sys
import os
import tempfile
import gzip
import tarfile
import glob
import subprocess
import shutil

try:
    from StringIO import StringIO
except ImportError:
    from io import BytesIO as StringIO

try:
    from subprocess import DEVNULL # py3k
except ImportError:
    import os
    DEVNULL = open(os.devnull, 'wb')

# Set up functions B and S to convert a string to bytes or vice versa. Useful
# for both python 2 and 3 support
if sys.version_info[0] == 2:
    B = lambda s: s
    S = B
else:
    B = lambda s: bytes(s, 'UTF-8')
    S = lambda b: b.decode(encoding='UTF-8')

def getVersionFromFilename(fileName):
    prefix = "simpact-cyan-"
    suffix = ".tar.gz"

    if fileName.startswith(prefix) and fileName.endswith(suffix):
        return fileName[len(prefix):-len(suffix)]

    raise Exception("Can't get version from " + fileName)

def getPackageData(fullFileName):
    if not os.path.exists(fullFileName): # assume we still need to download the file
        print("Downloading " + fullFileName)
        response = urlopen(fullFileName)
        data = response.read()
    else:
        print("Reading file " + fullFileName)
        with open(fullFileName, "rb") as f:
            data = f.read()

    return data

def createBuildAndSourceDir(path):

    workDir = tempfile.mkdtemp(prefix = "simpactbuildtmp")
    print("Work directory is " + workDir)

    if os.path.isdir(path):
        path = os.path.abspath(path)
        cmakeFile = os.path.join(path, "CMakeLists.txt")
        if not os.path.exists(cmakeFile):
            raise Exception("Directory specified, but no CMakeLists.txt file exists there")

        version = "unknownversion"
        for l in open(cmakeFile).readlines():
            vStr = "set(VERSION"
            if vStr in l:
                x = l[l.find(vStr)+len(vStr):]
                version = x.split(")")[0].strip()
                break

        srcDir = path

        os.chdir(workDir)
        os.mkdir("build")
        os.chdir("build")

        buildDir = os.getcwd()

    else:
        version = getVersionFromFilename(os.path.basename(path))

        data = getPackageData(path)
        gf = gzip.GzipFile(fileobj = StringIO(data), mode='r')
        uncompressedData = gf.read()

        tf = tarfile.open(fileobj = StringIO(uncompressedData))

        os.chdir(workDir)
        tf.extractall()

        # Go to the extracted directory
        os.chdir(glob.glob("*")[0])

        srcDir = os.getcwd()

        os.mkdir("build")
        os.chdir("build")

        buildDir = os.getcwd()

    print("Source directory is: ", srcDir)
    print("Building version: ", version)
    print("Build directory is: ", buildDir)

    return (buildDir, srcDir, version, workDir)
   
def compileCMakeIfNecessary(workDir):
    out = S(subprocess.check_output(["cmake", "--version"]))
    if out and len(out.splitlines()) > 0:
        version = [ x for x in map(int, out.splitlines()[0].split()[-1].split(".")) ]
        while len(version) < 4:
            version += [ 0 ]

        print("Current cmake version is {}".format(version))
        if version >= [ 2, 8, 12, 0 ]: # no rebuild necessary
            return "cmake"

    print("Compiling newer cmake")
    
    cmakeUrl = "https://cmake.org/files/v2.8/cmake-2.8.12.2.tar.gz"
    response = urlopen(cmakeUrl)
    data = response.read()

    gf = gzip.GzipFile(fileobj = StringIO(data), mode='r')
    uncompressedData = gf.read()

    tf = tarfile.open(fileobj = StringIO(uncompressedData))

    os.chdir(workDir)
    
    subDir = "cmakebuild"
    os.mkdir(subDir)
    os.chdir(subDir)
    tf.extractall()

    os.mkdir("inst")
    installPrefix = os.path.join(os.getcwd(), "inst")

    # Go to the extracted directory
    os.chdir(glob.glob("cmake*")[0])

    srcDir = os.getcwd()

    os.mkdir("build")
    os.chdir("build")

    subprocess.check_call([ "cmake", "-DCMAKE_BUILD_TYPE=release", "-DCMAKE_INSTALL_PREFIX={}".format(installPrefix), ".."])
    subprocess.check_call([ "make", "install" ])

    return os.path.join(installPrefix, "bin", "cmake")


def gccSupportsOverride(workDir):
    src = """
struct A
{
    virtual void f() { x = 10; }
    int x;
};

struct B : public A
{
    void f() override { x = 11; }
};

int main(void)
{
    B b;
    return 0;
}
"""
    os.chdir(workDir)

    fName = "overriddetest.cpp"
    open(fName, "wt").write(src)
    try:
        subprocess.check_call(["g++", "-c", "-o", fName + ".o", "-std=c++0x", fName])
        return True
    except subprocess.CalledProcessError:
        return False

def gccSupportsSteadyClock(workDir):
    src = """
#include <chrono>

int main(void)
{
    auto t0 = std::chrono::steady_clock::now();
    return 0;
}
"""
    os.chdir(workDir)

    fName = "steadyclocktest.cpp"
    open(fName, "wt").write(src)
    try:
        subprocess.check_call(["g++", "-c", "-o", fName + ".o", "-std=c++0x", fName])
        return True
    except subprocess.CalledProcessError:
        return False

def build(path, extraCMakeOpts):

    buildDir, srcDir, version, workDir = createBuildAndSourceDir(path)

    cmakeExe = compileCMakeIfNecessary(workDir)

    defines = ''
    defines += ' -Doverride=' if not gccSupportsOverride(workDir) else ''
    defines += ' -Dsteady_clock=system_clock' if not gccSupportsSteadyClock(workDir) else ''
    extraOpts = [ '-DCMAKE_CXX_FLAGS={}'.format(defines) ] if defines else [ ]

    print("Using extra CMake options: {}".format(extraOpts))

    os.chdir(buildDir)
    subprocess.check_call([cmakeExe, srcDir] + extraCMakeOpts + extraOpts)
    subprocess.check_call(["make"])

    return (workDir, buildDir, version)

def createProfileDEntry():
    os.mkdir("etc")
    os.mkdir("etc/profile.d")
    open("etc/profile.d/simpactcyan.sh", "wt").write("""

if [ -z "$PYTHONPATH" ] ; then
	PYTHONPATH=/usr/share/simpact-cyan/python
else
	PYTHONPATH="$PYTHONPATH":/usr/share/simpact-cyan/python
fi

export PYTHONPATH
""")

def checkCommandLineArguments():
    try:
        fileName = None
        options = set()
        for o in sys.argv[1:]:
            if o.startswith("--"):
                if o in [ "--noupdate", "--copypackage" ]:
                    options.add(o[2:])
                else:
                    raise Exception("Unknown option '{}'".format(o))
            else:
                if fileName is None:
                    fileName = o
                else:
                    raise Exception("Trying to set filename to '{}' but was already set to '{}'".format(o,fileName))
        
        if fileName is None:
            raise Exception("No path was specified")

    except Exception as e:
        print("""Error: {}

Usage: {} [--noupdate] [--copypackage] path

Here 'path' can be a local .tar.gz file, a .tar.gz file available over http
or the base directory of a project (containing the main CMakeLists.txt file).
""".format(e, sys.argv[0]))
        sys.exit(-1)

    return (fileName, [ o for o in options])

