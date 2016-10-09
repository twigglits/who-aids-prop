add_custom_command(OUTPUT config_tree.cpp 
	COMMAND embedfile configJSon ${CMAKE_CURRENT_SOURCE_DIR}/config_tree.json config_tree.cpp
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config_tree.json)

set(SOURCES_SIMPACT
	simpactpopulation.cpp
	person.cpp
	eventmortalitybase.cpp
	eventmortality.cpp
	eventaidsmortality.cpp
	eventformation.cpp
	eventdissolution.cpp
	eventdebut.cpp
	eventtransmission.cpp
	eventchronicstage.cpp
	eventtreatment.cpp
	eventhivseed.cpp
	eventintervention.cpp
	eventaidsstage.cpp
	eventconception.cpp
	eventbirth.cpp
	vspmodellogweibullwithnoise.cpp
	vspmodellogbinormal.cpp
	logsystem.cpp
	simpactevent.cpp
	evthazardformationsimple.cpp
	evthazardformationagegap.cpp
	hazardfunctionformationsimple.cpp
	hazardfunctionformationagegap.cpp
	main_hazardtest.cpp
	main.cpp
	config_tree.cpp
	configutil.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_simpact_executable(simpact-cyan ${SOURCES_SIMPACT})
install_simpact_executable(simpact-cyan)


