#!/usr/bin/env python

from __future__ import print_function
from common import *

def listFilesUnderDirectory(dirName):
    matches = []
    for root, dirnames, filenames in os.walk(dirName):
        for filename in filenames:
            matches.append(os.path.join(root, filename))

    return matches

def getFedoraVersion():
    return "fc" + open("/etc/fedora-release").read().splitlines()[0].split()[2]

def createFedoraPackage(workDir, buildDir, version):
    p = os.path.join(buildDir, "simpact-cyan-release")
    output = S(subprocess.check_output(["ldd", p ]))

    instDir = os.path.join(workDir, "BUILDROOT")
    os.mkdir(instDir)
    os.chdir(buildDir)
    subprocess.check_call([ "make", "install", "DESTDIR={}".format(instDir)])

    os.chdir(instDir)
    createProfileDEntry()

    fcVersion = getFedoraVersion()

    specFile = os.path.join(workDir, "simpactcyan.spec")
    with open(specFile, "wt") as f:
        f.write("""
Name:		simpactcyan
Version:	{}
Release:	1%{{?dist}}
Summary:	Simpact Cyan
Requires:	libtiff gsl
License:	GPL

%description
SimpactCyan programs for individual based simulations using the MNRM algorithm

%files
""".format(version))

        for n in listFilesUnderDirectory(instDir):
            if not n.startswith(instDir):
                raise Exception("Filename {} does not start with {}".format(n, instDir))
        
            f.write(n[len(instDir):] + "\n")

    print(open(specFile).read())

    out = S(subprocess.check_output(["rpmbuild", "-bb", "--buildroot=" + instDir, specFile]))
    for l in out.splitlines():
        pref = "Wrote:"
        if l.startswith(pref):
            return l[len(pref):].strip()

    return None

def main():

    startDir = os.getcwd()
    fullFileName, options = checkCommandLineArguments()

    if not "noupdate" in options:
        subprocess.call([ "sudo", "dnf", "update", "-y" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "rpm-build" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "cmake" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "make" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "gcc" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "gcc-c++" ])
        subprocess.call([ "sudo", "dnf", "install", "-y", "gsl-devel"])
        subprocess.call([ "sudo", "dnf", "install", "-y", "libtiff-devel"])

    workDir, buildDir, version = build(sys.argv[1], [ "-DCMAKE_INSTALL_PREFIX=/usr" ])
    outputFileName = createFedoraPackage(workDir, buildDir, version)
    print("Output file is: " + outputFileName)

    if "copypackage" in options:
        shutil.copy(outputFileName, os.path.join(startDir, os.path.basename(outputFileName)))

if __name__ == "__main__":
    main()
