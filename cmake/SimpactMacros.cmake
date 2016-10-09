macro(simpact_setup)
	include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

	if (NOT UNIX OR CMAKE_GENERATOR STREQUAL Xcode)
		set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
	endif()

	get_install_directory(LIBRARY_INSTALL_DIR)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

	# TODO: Just for some tests
	#find_package(MEANWALKER REQUIRED)
	#include_directories(${MEANWALKER_INCLUDE_DIR})

	find_package(ZLIB)
	include_directories(${ZLIB_INCLUDE_DIRS})

	find_package(GSL REQUIRED)
	include_directories(${GSL_INCLUDE_DIR})

	find_package(OpenMP)
	find_package(RT)

	find_package(TIFF REQUIRED)
	include_directories(${TIFF_INCLUDE_DIR})
	string(REPLACE "." ";" TIFF_VERSION_LIST "${TIFF_VERSION_STRING}")
	list(GET TIFF_VERSION_LIST 0 TIFF_VERSION_MAJOR)

	if (NOT RT_LIBRARIES)
		set(RT_LIBRARIES "")
	endif()

	add_additional_stuff(EXTRA_INCLUDES EXTRA_LIBS)

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

	include_directories(${PROJECT_SOURCE_DIR}/src/)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/simpact)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/mnrm)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/util)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/util/experimental)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/core)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib)
	include_directories(${PROJECT_BINARY_DIR}/)
	include_directories(${PROJECT_BINARY_DIR}/src)
	include_directories(${EXTRA_INCLUDES})

	configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h)

	set(SOURCES_UTIL
		${PROJECT_SOURCE_DIR}/src/lib/util/util.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/csvfile.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/mutex.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/discretedistribution.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/discretedistributionfast.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/discretedistribution2d.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/populationdistributioncsv.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/populationdistribution.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/configreader.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/configsettings.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/configdistributionhelper.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/experimental/discretefunction.cpp
		#${PROJECT_SOURCE_DIR}/src/lib/util/experimental/Faddeeva.cc
		#${PROJECT_SOURCE_DIR}/src/lib/util/experimental/inverseerfi.cpp
		#${PROJECT_SOURCE_DIR}/src/lib/util/experimental/exponentialfunction.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/hazardfunction.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/logfile.cpp 
		${PROJECT_SOURCE_DIR}/src/lib/util/tiffdensityfile.cpp 
		${PROJECT_SOURCE_DIR}/src/lib/util/configwriter.cpp 
		${PROJECT_SOURCE_DIR}/src/lib/util/piecewiselinearfunction.cpp 
		)
	set(SOURCES_MRNM
		${PROJECT_SOURCE_DIR}/src/lib/mnrm/gslrandomnumbergenerator.cpp
		${PROJECT_SOURCE_DIR}/src/lib/mnrm/state.cpp
		${PROJECT_SOURCE_DIR}/src/lib/mnrm/simplestate.cpp
		${PROJECT_SOURCE_DIR}/src/lib/mnrm/eventbase.cpp)
	set(SOURCES_CORE
		${PROJECT_SOURCE_DIR}/src/lib/core/personbase.cpp
		${PROJECT_SOURCE_DIR}/src/lib/core/population.cpp
		${PROJECT_SOURCE_DIR}/src/lib/core/populationevent.cpp
		${PROJECT_SOURCE_DIR}/src/lib/core/personaleventlist.cpp)

	source_group(util FILES ${SOURCES_UTIL})
	source_group(mnrm FILES ${SOURCES_MRNM})
	source_group(core FILES ${SOURCES_CORE})

	if (UNIX AND NOT CMAKE_GENERATOR STREQUAL Xcode)
		if (NOT CMAKE_BUILD_TYPE STREQUAL "")
			message(FATAL_ERROR "\n\nThe CMake variable CMAKE_BUILD_TYPE should be empty, versions with different settings will be created automatically!\n\n")
		endif()
	endif()

	set(SOURCES ${SOURCES_UTIL} ${SOURCES_CORE} ${SOURCES_MRNM})
	add_simpact_library("simpact-lib-static" ${SOURCES})

	# From: http://stackoverflow.com/questions/11813271/embed-resources-eg-shader-code-images-into-executable-library-with-cmake
	add_executable(embedfile "${PROJECT_SOURCE_DIR}/cmake/embedfile.c")
endmacro()

macro(add_simpact_library LIBPREFIX MAINSOURCES)
	set(MAINSOURCES0 "${ARGV}")
	list(REMOVE_AT MAINSOURCES0 0)
	set(SOURCES ${MAINSOURCES0})
	add_simpact_executable_or_library("LIB" ${LIBPREFIX} ${SOURCES})
endmacro()

macro(add_simpact_executable EXEPREFIX MAINSOURCES)
	set(MAINSOURCES0 "${ARGV}")
	list(REMOVE_AT MAINSOURCES0 0)
	set(SOURCES ${MAINSOURCES0})
	add_simpact_executable_or_library("EXE" ${EXEPREFIX} ${SOURCES})
endmacro()

macro(install_simpact_executable EXEPREFIX)
	if (UNIX AND NOT CMAKE_GENERATOR STREQUAL Xcode)
		install(TARGETS ${EXEPREFIX}-opt ${EXEPREFIX}-opt-debug ${EXEPREFIX}-basic ${EXEPREFIX}-basic-debug DESTINATION bin)
	else()
		install(TARGETS ${EXEPREFIX}-opt ${EXEPREFIX}-basic DESTINATION bin)
	endif()
endmacro()

macro(add_library_or_executable LIBOREXEFLAG LIBOREXENAME MAINSOURCES)
	#message(${LIBOREXEFLAG})
	#message(${LIBOREXENAME})
	#message(${SOURCES})
	set(MAINSOURCES0 "${ARGV}")
	list(REMOVE_AT MAINSOURCES0 0)
	list(REMOVE_AT MAINSOURCES0 0)
	set(SOURCES ${MAINSOURCES0})

	if (${LIBOREXEFLAG})
		#message("Adding library")
		add_library(${LIBOREXENAME} STATIC ${SOURCES})
	else()
		#message("Adding executable")
		add_executable(${LIBOREXENAME} ${SOURCES})
	endif()
endmacro()

macro(add_simpact_executable_or_library LIBOREXE EXEPREFIX MAINSOURCES)
	#message(${LIBOREXE})
	set(MAINSOURCES0 "${ARGV}")
	list(REMOVE_AT MAINSOURCES0 0)
	list(REMOVE_AT MAINSOURCES0 0)
	set(SOURCES ${MAINSOURCES0})

	string(COMPARE EQUAL ${LIBOREXE} "LIB" USELIBSETTINGS)
	#message(${USELIBSETTINGS})

	if (OPENMP_FOUND)
		set(OPENMPDEFINE "")
	else()
		set(OPENMPDEFINE "DISABLEOPENMP")
	endif()

	set(ALLLIBS ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES} ${MEANWALKER_LIBRARIES} ${TIFF_LIBRARIES})

	if (UNIX AND NOT CMAKE_GENERATOR STREQUAL Xcode)
		if (USELIBSETTINGS)
			#message("Using library settings")
			set(ALLLIBSOPT ${ALLLIBS})
			set(ALLLIBSOPTDEBUG ${ALLLIBS})
			set(ALLLIBSBASIC ${ALLLIBS})
			set(ALLLIBSBASICDEBUG ${ALLLIBS})
		else()
			#message("Using exe settings")
			set(ALLLIBSOPT simpact-lib-static-opt)
			set(ALLLIBSOPTDEBUG simpact-lib-static-opt-debug)
			set(ALLLIBSBASIC simpact-lib-static-basic)
			set(ALLLIBSBASICDEBUG simpact-lib-static-basic-debug)
		endif()

		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-opt ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt ${ALLLIBSOPT})
		set_target_properties(${EXEPREFIX}-opt PROPERTIES COMPILE_DEFINITIONS "TIFFVERSION=${TIFF_VERSION_MAJOR};${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		set_target_properties(${EXEPREFIX}-opt PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		set_target_properties(${EXEPREFIX}-opt PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		add_openmp_flags(${EXEPREFIX}-opt) # Must be last (set_target_properties changes it again otherwise)

		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-basic ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic ${ALLLIBSBASIC})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_DEFINITIONS "SIMPLEMNRM;TIFFVERSION=${TIFF_VERSION_MAJOR};${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		add_openmp_flags(${EXEPREFIX}-basic)

		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-opt-debug ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt-debug ${ALLLIBSOPTDEBUG})
		set_target_properties(${EXEPREFIX}-opt-debug PROPERTIES COMPILE_DEFINITIONS "TIFFVERSION=${TIFF_VERSION_MAJOR};${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		set_target_properties(${EXEPREFIX}-opt-debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		set_target_properties(${EXEPREFIX}-opt-debug PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		add_openmp_flags(${EXEPREFIX}-opt-debug)

		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-basic-debug ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic-debug simpact-lib-static-basic-debug ${ALLLIBSBASICDEBUG})
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES COMPILE_DEFINITIONS "TIFFVERSION=${TIFF_VERSION_MAJOR};SIMPLEMNRM;${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		add_openmp_flags(${EXEPREFIX}-basic-debug)
	else()

		if (USELIBSETTINGS)
			set(ALLLIBSOPT ${ALLLIBS})
			set(ALLLIBSBASIC ${ALLLIBS})
		else()
			set(ALLLIBSOPT simpact-lib-static-opt)
			set(ALLLIBSBASIC simpact-lib-static-basic)
		endif()
		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-opt ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt ${ALLLIBSOPT})
		set_target_properties(${EXEPREFIX}-opt PROPERTIES COMPILE_DEFINITIONS "TIFFVERSION=${TIFF_VERSION_MAJOR};${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		add_openmp_flags(${EXEPREFIX}-opt) # Must be last (set_target_properties changes it again otherwise)

		add_library_or_executable(${USELIBSETTINGS} ${EXEPREFIX}-basic ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic ${ALLLIBSBASIC})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_DEFINITIONS "SIMPLEMNRM;TIFFVERSION=${TIFF_VERSION_MAJOR};${OPENMPDEFINE};EVENTBASE_ALWAYS_CHECK_NANTIME")
		add_openmp_flags(${EXEPREFIX}-basic) # Must be last (set_target_properties changes it again otherwise)

		if (USELIBSETTINGS)
			set(CMAKE_DEBUG_POSTFIX "-debug") # Is only used for libraries
		else()
			set_property(TARGET ${EXEPREFIX}-opt PROPERTY DEBUG_OUTPUT_NAME "${EXEPREFIX}-opt-debug")
			set_property(TARGET ${EXEPREFIX}-basic PROPERTY DEBUG_OUTPUT_NAME "${EXEPREFIX}-basic-debug")
		endif()
	endif()
endmacro(add_simpact_executable_or_library)
