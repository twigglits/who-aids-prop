#!/usr/bin/env python

from __future__ import print_function
from common import *
import sys
import os
import glob
import shutil
import bz2

def buildPackageUsingConda(data, fileName, version):

    tmpDir = tempfile.mkdtemp(prefix = "simpactbuildtmp")
    os.chdir(tmpDir)
    print("Work directory is " + tmpDir)

    fullFileName = os.path.join(tmpDir, fileName)
    open(fullFileName, "wb").write(data)

    os.mkdir("condapack")
    open("condapack/meta.yaml", "wt").write("""
package:
  name: simpactcyan
  version: "{}"

source:
  fn: {}
  url: file://{}

build:
  number: 0

requirements:
  build:
    - clangxx_osx-64
    - llvm-openmp
    - cmake     
    - gsl
    - libtiff 
    - python

  run:
    - gsl
    - libtiff
    - llvm-openmp
        
""".format(version, fileName, fullFileName))

    open("condapack/build.sh","wt").write("""
#!/bin/bash

unset MACOSX_DEPLOYMENT_TARGET
mkdir build && cd build && CFLAGS="-L $PREFIX/lib -I $PREFIX/include -I $PREFIX/clang/*/include/" && CXXFLAGS="-L $PREFIX/lib -I $PREFIX/include -I $PREFIX/clang/*/include/" cmake .. -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_FIND_ROOT_PATH="$PREFIX" && make && make install
""")

    open("condapack/bld.bat", "wt").write("")

    # This builds the executables
    subprocess.check_call( ["conda", "build", "-c", "jori", "condapack" ])

    return tmpDir

def createDmg(tmpDir, version, startDir):

    envName = "simpactenvdir" # TODO: something more random?
    print("Using conda environment name", envName)

    # Get the conda dir
    condaDir = os.environ["CONDA_PREFIX"]
    if not condaDir:
        raise Exception("Couldn't detect conda directory")

    packageFile = glob.glob(os.path.join(condaDir, "conda-bld", "osx-64", "simpactcyan-" + version + "-*.tar.bz2"))[0]
    print("Package file is " + packageFile)

    condaDir = os.path.join(condaDir, "envs", envName) # Install everything in separate env, seems to be needed to get dependencies just right
    if os.path.exists(condaDir):
        raise Exception("Environment", envName, "already exists")

    subprocess.check_call( ["conda", "create", "-y", "-n", envName, "--use-local", "-c", "jori", "simpactcyan"])

    shutil.copytree(os.path.join(startDir, "platform-osx"), os.path.join(tmpDir, "packages-osx"))

    projectFileData = open(os.path.join(tmpDir, "packages-osx", "SimpactCyan.pkgproj.templ"), "rt").read()
    projectFileData = projectFileData.replace("SIMPACTCYANPROJECTBASEPATH", tmpDir)
    open(os.path.join(tmpDir, "packages-osx", "SimpactCyan.pkgproj"), "wt").write(projectFileData)

    bf = bz2.BZ2File(packageFile, mode="r")
    uncompressedData = bf.read()
    tf = tarfile.open(fileobj = StringIO(uncompressedData))

    os.chdir(tmpDir)
    tf.extractall()

    for exe in glob.glob("bin/*"):
        shutil.copy(exe, "packages-osx/Applications/SimpactCyan.app/Contents/bin/" + os.path.basename(exe))

    for dat in glob.glob("share/simpact-cyan/*"):
        if not os.path.isdir(dat):
            shutil.copy(dat, "packages-osx/Applications/SimpactCyan.app/Contents/data/" + os.path.basename(dat))

    shutil.copy("share/simpact-cyan/python/pysimpactcyan.py", os.path.join("packages-osx", "Applications", "SimpactCyan.app", "Contents", "python", "pysimpactcyan.py"))

    print("Copying executables from conda build and changing library locations")
    # Copy the used libraries from conda to the 'lib' dir
    os.chdir("packages-osx/Applications/SimpactCyan.app/Contents/bin")
    for exeName in glob.glob("*"):
        out = S(subprocess.check_output(["otool", "-L", exeName]))
        for l in out.splitlines():
            if "@rpath" in l:
                rpathLibName = l.split()[0]
                libName = os.path.basename(rpathLibName)

                shutil.copy(os.path.join(condaDir, "lib", libName), os.path.join("..", "lib", libName))
                subprocess.check_call(["install_name_tool", "-change", rpathLibName, "@executable_path/../lib/" + libName, exeName])

    print("Copying additional needed libraries from conda build")
    # Do the same for the libraries
    os.chdir("../lib")
    count = 20
    found = True
    while count > 0 and found:
        found = False
        count -= 1
        for exeName in glob.glob("*"):
            out = S(subprocess.check_output(["otool", "-L", exeName]))
            for l in out.splitlines():
                if "@rpath" in l and not exeName in l:
                    rpathLibName = l.split()[0]
                    libName = os.path.basename(rpathLibName)

                    if not os.path.exists(libName):
                        shutil.copy(os.path.join(condaDir, "lib", libName), libName)
                    
                    found = True
                    subprocess.check_call(["install_name_tool", "-change", rpathLibName, "@executable_path/../lib/" + libName, exeName])

    os.chdir(os.path.join(tmpDir, "packages-osx"))
    subprocess.check_call("./createiconfile.sh")
    subprocess.check_call("./addicon.sh")
    subprocess.check_call(["packagesbuild", "SimpactCyan.pkgproj"])
    subprocess.check_call(["./addicon2.sh"])

    os.chdir("build")
    subprocess.check_call(["hdiutil", "create", "SimpactCyan-{}.dmg".format(version), "-volname", "SimpactCyan-{}".format(version), "-fs", "HFS+", "-srcfolder", "SimpactCyan.mpkg"])

    destFile = os.path.join(os.getcwd(), "SimpactCyan-{}.dmg".format(version))
    return destFile

def main():

    startDir = os.getcwd()
    if not os.path.exists("platform-osx"):
        raise Exception("Need to be started from the directory which has 'platform-osx' as subdirectory")

    try:
        fullFileName = sys.argv[1]
        if len(sys.argv) > 2:
            raise Exception("Too many arguments")
    except Exception as e:
        print("""Error: {}

Usage: {} path

Here 'path' can be a local .tar.gz file or a .tar.gz file available over http.
""".format(e, sys.argv[0]))
        sys.exit(-1)

    try:
        print("Checking if 'packagesbuild' exists")
        subprocess.check_call(["packagesbuild"])
    except subprocess.CalledProcessError:
        pass
    except Exception as e:
        print("""Error: {}

Couldn't run 'packagesbuild', which is needed to create a dmg file.
Install http://s.sudre.free.fr/Software/Packages/about.html
""".format(e))
        sys.exit(-1)

    try:
        print("Checking if 'conda' can be run")
        subprocess.check_output([ "conda", "--version" ])
    except OSError as e:
        print("""Error: {}

The OS X package creation program relies on the Anaconda Python environment
to build the executables (to be able to use gcc instead of clang). Make sure 
this is installed and that the 'conda' command can be run from the command 
line.
""".format(e))
        sys.exit(-1)

    subprocess.call(["conda", "install", "-y", "conda-build"])

    fileName = os.path.basename(fullFileName)
    version = getVersionFromFilename(fileName)

    print("Version: " + version)

    data = getPackageData(fullFileName)
    tmpDir = buildPackageUsingConda(data, fileName, version)

    destFile = createDmg(tmpDir, version, startDir)
    print("")
    print("Package is " + destFile)


if __name__ == "__main__":
    main()
