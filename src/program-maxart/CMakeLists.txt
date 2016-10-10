set(SOURCES_SIMPACT
	personimpl.cpp
	facilities.cpp
	start.cpp
	maxartpopulation.cpp
	eventstudystart.cpp
	eventstudystep.cpp
	eventstudyend.cpp
	eventmonitoring.cpp
	../program-common/logsystem.cpp
	../program-common/person.cpp
	../program-common/simpactpopulation.cpp
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
	../program-common/eventdiagnosis.cpp
	../program-common/eventdropout.cpp
	../program-common/eventperiodiclogging.cpp
	../program-common/eventsyncpopstats.cpp
	../program-common/vspmodellogweibullwithnoise.cpp
	../program-common/vspmodellogdist.cpp
	../program-common/simpactevent.cpp
	../program-common/evthazardformationsimple.cpp
	../program-common/evthazardformationagegap.cpp
	../program-common/hazardfunctionformationsimple.cpp
	../program-common/hazardfunctionformationagegap.cpp
	../program-common/main_hazardtest.cpp
	../program-common/main.cpp
	../program-common/signalhandlers.cpp
	../program-common/configutil.cpp
	../program-common/aidstodutil.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../program-common/")
add_simpact_executable(maxart ${SOURCES_SIMPACT})
install_simpact_executable(maxart)

