add_custom_command(OUTPUT config_tree.cpp 
	COMMAND embedfile configJSon ${CMAKE_CURRENT_SOURCE_DIR}/config_tree.json config_tree.cpp
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config_tree.json)

set(SOURCES_SIMPACT
	simpactpopulation.cpp
	person.cpp
	logsystem.cpp
	config_tree.cpp
	../program-common/eventmortalitybase.cpp
	../program-common/eventmortality.cpp
	../program-common/eventaidsmortality.cpp
	../program-common/eventformation.cpp
	../program-common/eventdissolution.cpp
	../program-common/eventdebut.cpp
	../program-common/eventtransmission.cpp
	../program-common/eventchronicstage.cpp
	../program-common/eventhivseed.cpp
	../program-common/eventintervention.cpp
	../program-common/eventaidsstage.cpp
	../program-common/eventconception.cpp
	../program-common/eventbirth.cpp
	../program-common/eventtest.cpp
	../program-common/eventdropout.cpp
	../program-common/vspmodellogweibullwithnoise.cpp
	../program-common/vspmodellogbinormal.cpp
	../program-common/simpactevent.cpp
	../program-common/evthazardformationsimple.cpp
	../program-common/evthazardformationagegap.cpp
	../program-common/hazardfunctionformationsimple.cpp
	../program-common/hazardfunctionformationagegap.cpp
	../program-common/main_hazardtest.cpp
	../program-common/main.cpp
	../program-common/configutil.cpp
	../program-common/aidstodutil.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../program-common/")
add_simpact_executable(maxart ${SOURCES_SIMPACT})
install_simpact_executable(maxart)

