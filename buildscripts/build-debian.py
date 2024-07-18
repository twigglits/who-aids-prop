#!/usr/bin/env python

from __future__ import print_function
from common import *

def getArchitecture():
    out = S(subprocess.check_output(["uname", "-a"]))
    if "x86_64" in out:
        return "amd64"
    return "i386"

def getLibraryPackage(libName):
    out = S(subprocess.check_output(["dpkg-query", "-S", libName]))
    for l in out.splitlines():
        parts = l.split()
        path = parts[1]
        print(parts)
        if os.path.exists(path) and not os.path.islink(path):
            pack = l.split(":")[0]
            print("Package is: " + pack)
            return pack

    raise Exception("Package for {} not found".format(libName))

def getPlatformAndVersion():
    out = S(subprocess.check_output(["lsb_release", "-a"], stderr=DEVNULL))
    platform = "unknown"
    platformVersion = "unknown"
    for l in out.splitlines():
        if "Distributor ID" in l:
            platform = l.split("\t")[1].lower()
        elif "Release" in l:
            platformVersion = l.split("\t")[1].lower()

    return (platform, platformVersion)

def createDebianPackage(workDir, buildDir, version):
    p = os.path.join(buildDir, "simpact-cyan-release")
    output = S(subprocess.check_output(["ldd", p ]))

    libTiff = getLibraryPackage("libtiff.so")
    libGsl = getLibraryPackage("libgsl.so")
    arch = getArchitecture()

    print("TIFF package: " + libTiff)
    print("GSL package: " + libGsl)
    print("Architecture: " + arch)

    instDir = os.path.join(workDir, "fakeinst")
    os.mkdir(instDir)
    os.chdir(buildDir)
    subprocess.check_call([ "make", "install", "DESTDIR={}".format(instDir)])

    os.chdir(instDir)
    createProfileDEntry()

    origFiles = glob.glob("*")
    lines = S(subprocess.check_output(["du" ] + origFiles + [ "--max-depth=0" ])).splitlines()
    totalSize = 0
    for l in lines:
        totalSize += int(l.split()[0])

    print("Total size: ", totalSize)

    subprocess.check_call(["tar", "cfzv", "data.tar.gz" ] + origFiles)
    
    # Remove the original files themselves
    for i in origFiles:
        subprocess.check_call([ "rm", "-rf", i ])

    open("debian-binary", "wt").write("2.0\n")
    open("control", "wt").write("""Package: simpactcyan
Version: {}
Section: user
Priority: optional
Architecture: {}
Installed-Size: {}
Maintainer: Jori Liesenborgs <jori.liesenborgs@uhasselt.be>
Description: SimpactCyan programs for individual based simulations using the MNRM algorithm
Depends: {}, {}, libgomp1
""".format(version, arch, totalSize, libGsl, libTiff))

    subprocess.check_call(["tar", "cfzv", "control.tar.gz", "control"])
    subprocess.check_call(["ar", "-r", "simpactcyan.deb", "debian-binary", "control.tar.gz", "data.tar.gz" ])

    platform, platformVersion = getPlatformAndVersion()

    outputFileName = "simpactcyan-{}-{}-{}-{}.deb".format(version, platform, platformVersion, arch)
    shutil.move("simpactcyan.deb", outputFileName)

    return outputFileName

def main():

    startDir = os.getcwd()
    fullFileName, options = checkCommandLineArguments()

    if not "noupdate" in options:
        subprocess.call([ "sudo", "apt-get", "update", "-y" ])
        # subprocess.call([ "sudo", "apt-get", "upgrade", "-y" ])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "cmake", "make", "gcc", "g++"])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "libgsl-dev"])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "libgsl0-dev"])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "libtiff-dev"])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "libtiff4-dev"])
        subprocess.call([ "sudo", "apt-get", "install", "-y", "libtiff5-dev"])

    # We also disable the 'override' compiler check this way, so that it also
    # builds using an older compiler

    workDir, buildDir, version = build(fullFileName, [ "-DCMAKE_INSTALL_PREFIX=/usr" ])
    outputFileName = createDebianPackage(workDir, buildDir, version)
    print("Output file is: " + outputFileName)

    if "copypackage" in options:
        shutil.copy(outputFileName, os.path.join(startDir, os.path.basename(outputFileName)))

if __name__ == "__main__":
    main()
