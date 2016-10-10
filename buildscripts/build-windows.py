#!/usr/bin/env python

from __future__ import print_function
from common import *
import subprocess
import string
import glob
from ctypes import windll

# From stackoverflow
def getAvailableDrives():
    drives = []
    bitmask = windll.kernel32.GetLogicalDrives()
    for letter in string.ascii_uppercase:
        if bitmask & 1:
            drives.append(letter)
        bitmask >>= 1

    return drives

def getPathHelper(exeName, possiblePaths):
    drives = getAvailableDrives()
    
    for d in drives:
        for p in possiblePaths:
            path = d + ":" + p
            fName = os.path.join(path, exeName)
            if os.path.exists(fName):
                return path

    return None

def getVCVarsPath():

    # Check if msbuild is already in the path
    try:
        subprocess.check_call(["msbuild", "/version"], stderr = DEVNULL, stdout = DEVNULL)
        return None # Nothing needs to done
    except:
        pass

    path = getPathHelper("vcvarsall.bat", [ r"\Program Files (x86)\Microsoft Visual Studio 14.0\VC" ])
    if not path:
        raise Exception("msbuild.exe is not in PATH, and cannot locate path of vcvarsall.bat (from VS2015).")
    return os.path.join(path, "vcvarsall.bat")

def getCMakePath():

    # Check if it's in the PATH
    try:
        subprocess.check_call(["cmake", "--version"], stderr = DEVNULL, stdout = DEVNULL)
        return None # Nothing needs to be added to the path
    except:
        pass

    # See if we can locate the executable
    path = getPathHelper("cmake.exe", [ r"\Program Files (x86)\CMake\bin" ])
    if not path:    
        raise Exception("cmake.exe is not in PATH, and cannot locate it.")
    return path

def getNSISPath():

    # Check if it's in the PATH
    try:
        subprocess.check_call(["makensis", "/VERSION"], stderr = DEVNULL, stdout = DEVNULL)
        return None # Nothing needs to be added to the path
    except:
        pass

    path = getPathHelper("makensis.exe", [ r"\Program Files (x86)\NSIS" ])
    if not path:    
        raise Exception("makensis.exe is not in PATH, and cannot locate it.")
    return path

def getGslAndTiffLibPrefix():

    def existsInPath(path, files):
        path = os.path.abspath(path)
        for n in files:
            if not os.path.exists(os.path.join(path, n)):
                return False

        return True


    paths = os.environ["PATH"].split(os.pathsep)
    for p in paths:
        if existsInPath(p, [ "gsl.dll", "cblas.dll", "tiff.dll" ]):
            upPath = os.path.abspath(os.path.join(p, ".."))
            libPath = os.path.join(upPath, "lib")
            incPath = os.path.join(upPath, "include")
            incGslPath = os.path.join(incPath, "gsl")
            if existsInPath(libPath, [ "gsl.lib", "cblas.lib", "tiff.lib" ]) and \
                    existsInPath(incPath, [ "tiff.h" ]) and \
                    existsInPath(incGslPath, [ "gsl_rng.h" ]):
                return upPath

    raise Exception("Unable to locate directory with GSL and TIFF libraries. Make sure that the files are installed and that the 'bin' direcory with dll files have been added to the PATH variable.")

def makeNSISFile(buildDir, srcDir, version, libDir):

    open(os.path.join(buildDir, "setpythonpath.bat"), "wt").write("""
setx PYTHONPATH "%PYTHONPATH%";%1
""")

    nsisStart = r"""
; The name of the installer
Name "SimpactCyan"

; The file to write
OutFile "SimpactCyan-{v}-install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\SimpactCyan

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Installer section" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File {b}\Release\simpact-cyan-release.exe
  File {b}\Debug\simpact-cyan-debug.exe
  File {b}\Release\maxart-release.exe
  File {b}\Debug\maxart-debug.exe
  File {l}\cblas.dll  
  File {l}\gsl.dll
  File {l}\tiff.dll
  WriteUninstaller uninstall.exe

  File {b}\setpythonpath.bat

  SetOutPath $INSTDIR\data
"""

    nsisEnd = r"""
  SetOutPath $INSTDIR\python
  File {s}\python\pysimpactcyan.py

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SimpactCyan" "DisplayName" "SimpactCyan executables"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SimpactCyan" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""

  Exec '$INSTDIR\setpythonpath.bat "$INSTDIR\python"'
SectionEnd ; end the section

Section "un.Installer section"
  Delete $INSTDIR\setpythonpath.bat
  Delete $INSTDIR\simpact-cyan-release.exe
  Delete $INSTDIR\simpact-cyan-debug.exe
  Delete $INSTDIR\maxart-release.exe
  Delete $INSTDIR\maxart-debug.exe
  Delete $INSTDIR\cblas.dll  
  Delete $INSTDIR\gsl.dll
  Delete $INSTDIR\tiff.dll
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\python\pysimpactcyan.py
  RMDir /r $INSTDIR\data
  RMDir $INSTDIR\python
  RMDir $INSTDIR
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SimpactCyan"
SectionEnd
"""

    def getDataFiles(srcDir):
        startDir = os.getcwd()
        try:
            os.chdir(os.path.join(srcDir, "data"))
            return glob.glob("*") # TODO: for now there aren't any subdirs, so this might need to change later
        finally:
            os.chdir(startDir)

    dataFiles = getDataFiles(srcDir)
    nsisMiddle = ""
    for d in dataFiles:
        nsisMiddle += "  File {s}\\data\\" + d + "\n"

    nsiData = nsisStart + nsisMiddle + nsisEnd

    d = {
        "s": srcDir,
        "b": buildDir,
        "l": libDir,
        "v": version
    }

    nsiData = nsiData.format(**d)

    open(os.path.join(buildDir, "simpact.nsi"), "wt").write(nsiData)

def main():
    cmakePath = getCMakePath()
    print("CMake path is", cmakePath)

    vcvpath = getVCVarsPath()
    print("vcvarsall.bat path is", vcvpath)

    libsPrefix = getGslAndTiffLibPrefix()
    print("GSL and TIFF library prefix is", libsPrefix)

    nsisPath = getNSISPath()
    print("NSIS path is", nsisPath)

    path = sys.argv[1]
    buildDir, srcDir, version, workDir = createBuildAndSourceDir(path)

    makeNSISFile(buildDir, srcDir, version, os.path.join(libsPrefix, "bin"))

    batFile = os.path.join(buildDir, "buildit.bat")
    with open(batFile, "wt") as f:
        if cmakePath:
            f.write("set PATH=%PATH%;{}\n".format(cmakePath))
        if vcvpath:
            f.write('call "{}" x86\n'.format(vcvpath))
        if nsisPath:
            f.write("set PATH=%PATH%;{}\n".format(nsisPath))

        f.write('cmake -DCMAKE_INSTALL_PREFIX="{}" -G"Visual Studio 14 2015" -DCMAKE_CXX_FLAGS="-Dgsl_rng_get_default=gsl_rng_env_setup -DWIN32 /EHsc" "{}"\n'.format(libsPrefix, srcDir))
        f.write('if %errorlevel% neq 0 exit /b %errorlevel%\n')
        f.write('msbuild simpact-cyan.sln /p:Configuration=Release\n')
        f.write('if %errorlevel% neq 0 exit /b %errorlevel%\n')
        f.write('msbuild simpact-cyan.sln /p:Configuration=Debug\n')
        f.write('if %errorlevel% neq 0 exit /b %errorlevel%\n')
        f.write('makensis simpact.nsi\n')
        f.write('if %errorlevel% neq 0 exit /b %errorlevel%\n')
        
    os.chdir(buildDir)
    subprocess.check_call([batFile])

    print("")
    print("")
    print("")

    outputFile = os.path.join(buildDir, "SimpactCyan-{}-install.exe".format(version))
    if os.path.exists(outputFile):
        print("Output file is", outputFile)
    else:
        print("Build seems to have worked, or no error was detected, but output exe could not be located")

if __name__ == "__main__":
    main()
