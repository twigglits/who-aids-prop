Simpact Cyan
============

Simpact Cyan is the C++ version of the [Simpact](http://www.simpact.org/) family 
of programs. It is an agent based model (ABM) to study the way an infection 
spreads and can be influenced, and is currently focused on HIV. 

The main documentation can be found at [readthedocs](http://simpactcyan.readthedocs.io/).

Source code packages and pre-compiled versions can be found [here](http://research.edm.uhasselt.be/jori/simpact/programs/).

Compiling Simpact Cyan on MS-Windows
------------------------------------

Make sure you use Visual Studio 2015. In the following I'm assuming a 32-bit
build (as this also works on 64-bit systems), but a 64-bit build should work
as well, with some obvious changes to the commands.

 1. The source code needs GSL and TIFF libraries to build; to speed things up
    pre-compiled versions can be downloaded from: 
    [simpact_vs2015_deps.rar](http://research.edm.uhasselt.be/jori/simpact_vs2015_deps.rar).
    Extract these somewhere, below I'll assume that after extraction you have
    a directory called `c:\simpact_vs2015_deps\32bit`. If you've extracted the
    RAR archive somewhere else you'll need to change some paths in the instructions
    below.

 2. To be sure that the DLL files for the GSL and TIFF libraries can be found
    when executing the generated Simpact Cyan executable later on, add the
    `c:\simpact_vs2015_deps\32bit\bin` directory to the PATH environment
    variable (see e.g. [this link](http://www.computerhope.com/issues/ch000549.htm)
    if you don't know how to do this).

 3. Download the Simpact Cyan source code and extract it somewhere, or clone
    the GitHub repository somewhere. Below, I'll assume that this has created
    a path called `c:\projects\simpactcyan`.

 4. In that path, there's a file called `CMakeLists.txt`. Using a tool called
    [CMake](https://cmake.org/), this file will be analyzed and a Visual Studio
    project will be created. Make sure CMake is installed and start the GUI.

 5. In the top part of the window, you need to fill in where the source code
    is, and where to build the binaries. In the source code field, fill in
    `c:\projects\simpactcyan`. The build directory is arbitrary, that's just
    where the compiled files and executables will be stored, I typically use
    something like `c:\projects\simpactcyan\build`.

 6. We want to make sure that CMake can find the GSL and TIFF libraries, which
    is why we'll do the following: click the `Add Entry` button, enter
    `CMAKE_INSTALL_PREFIX` where it says `Name`, specity `PATH` as the `Type`,
    set the `Value` field to `c:\simpact_vs2015_deps\32bit` and click `Ok`.
    In the central part of the window, you'll see this `CMAKE_INSTALL_PREFIX`
    appear.

 7. In the main CMake GUI, press the button `Configure` and you'll get a 
    window that asks to specify the generator for the project. There, you should 
    be able to select `Visual Studio 14 2015` (the version number of Visual 
    Studio 2015 is 14). 
    
 8. If everything works correctly, you'll see a bunch of red lines appear in the
    same part of the window where the `CMAKE_INSTALL_PREFIX` was shown. If you
    click `Configure` again, they should become white, and the `Generate` button 
    should be possible to click. If there are still some red lines, it's possible 
    that you have to press `Configure` once more. After clicking `Generate`,
    a file called `c:\projects\simpact\build\simpact-cyan.sln` should be created.

 9. Open this `.sln` file (the 'solution' file, which is the name for a
    project in Visual Studio) with Visual Studio 2015. When this has been started,
    we'll be able to actually compile the source code and build the executables.

 10. By default, you'll see that `Debug` option is selected in the bar below the
    menu bar. This means that the debug version of the code will be compiled. You 
    can also change this to `Release` (and even some other things), which will 
    compile the optimized version of the code. For now, we'll just leave this at
    `Debug`.

 11. In the solution explorer, probably at the right of the Visual Studio window,
    you'll see the `ALL_BUILD` subproject. If you right-click this, you'll get
    a pop-up menu where the top item is `Build`. Click this to start the
    compilation process.

 12. If all went well, the folder `c:\projects\simpactcyan\build\Debug` should
    now contain the executable called `simpact-cyan-debug.exe`. Since this
    is a command line program, we'll start it from the command line: open
    the 'Command Prompt' and go to the specified directory. If you then
    type `simpact-cyan-debug`, the program should output some usage information.

 13. To compile the release version, simply change the the selection from step
    10, right click `ALL_BUILD` again and select `Build`.


