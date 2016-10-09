macro(simpact_setup)
	include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

	if (NOT UNIX)
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

	find_package(OpenMP REQUIRED)
	find_package(RT)

	if (NOT RT_LIBRARIES)
		set(RT_LIBRARIES "")
	endif()

	add_additional_stuff(EXTRA_INCLUDES EXTRA_LIBS)

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

	include_directories(${PROJECT_SOURCE_DIR}/src/lib/simpact)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/mnrm)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/util)
	include_directories(${PROJECT_SOURCE_DIR}/src/lib/core)
	include_directories(${PROJECT_BINARY_DIR}/)
	include_directories(${EXTRA_INCLUDES})

	set(SOURCES_UTIL
		${PROJECT_SOURCE_DIR}/src/lib/util/util.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/csvfile.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/mutex.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/discretedistribution.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/populationdistributioncsv.cpp
		${PROJECT_SOURCE_DIR}/src/lib/util/populationdistribution.cpp)
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

	if (UNIX)
		if (NOT CMAKE_BUILD_TYPE STREQUAL "")
			message(FATAL_ERROR "\n\nThe CMake variable CMAKE_BUILD_TYPE should be empty, versions with different settings will be created automatically!\n\n")
		endif()
	endif()

endmacro()

macro(add_simpact_executable EXEPREFIX MAINSOURCES)
	set(MAINSOURCES0 "${ARGV}")
	list(REMOVE_AT MAINSOURCES0 0)
	set(SOURCES ${MAINSOURCES0} ${SOURCES_UTIL} ${SOURCES_MRNM} ${SOURCES_CORE})
	if (UNIX)
		add_executable(${EXEPREFIX}-opt ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		set_target_properties(${EXEPREFIX}-opt PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		set_target_properties(${EXEPREFIX}-opt PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		add_openmp_flags(${EXEPREFIX}-opt) # Must be last (set_target_properties changes it again otherwise)

		add_executable(${EXEPREFIX}-basic ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_DEFINITIONS "SIMPLEMNRM")
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
		add_openmp_flags(${EXEPREFIX}-basic)

		add_executable(${EXEPREFIX}-opt-debug ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt-debug ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		set_target_properties(${EXEPREFIX}-opt-debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		set_target_properties(${EXEPREFIX}-opt-debug PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		add_openmp_flags(${EXEPREFIX}-opt-debug)

		add_executable(${EXEPREFIX}-basic-debug ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic-debug ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES COMPILE_DEFINITIONS "SIMPLEMNRM")
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		set_target_properties(${EXEPREFIX}-basic-debug PROPERTIES LINK_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
		add_openmp_flags(${EXEPREFIX}-basic-debug)
	else()
		add_executable(${EXEPREFIX}-opt ${SOURCES})
		target_link_libraries(${EXEPREFIX}-opt ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		add_openmp_flags(${EXEPREFIX}-opt) # Must be last (set_target_properties changes it again otherwise)

		add_executable(${EXEPREFIX}-basic ${SOURCES})
		target_link_libraries(${EXEPREFIX}-basic ${EXTRA_LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES} ${ZLIB_LIBRARIES} ${RT_LIBRARIES} ${JTHREAD_LIBRARIES})
		set_target_properties(${EXEPREFIX}-basic PROPERTIES COMPILE_DEFINITIONS "SIMPLEMNRM")
		add_openmp_flags(${EXEPREFIX}-basic) # Must be last (set_target_properties changes it again otherwise)
	endif()

endmacro(add_simpact_executable)
