set(SOURCES_SIMPACT
	simpactpopulation.cpp
	person.cpp
	eventmortality.cpp
	eventformation.cpp
	eventdissolution.cpp
	eventdebut.cpp
	eventbirth.cpp
	eventconception.cpp
	eventtransmission.cpp
	eventhivtest.cpp
	eventmtctransmission.cpp
	eventstopbreastfeeding.cpp
	eventchronicstage.cpp
	hazardutility.cpp
	main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_simpact_executable(simpact-cyan ${SOURCES_SIMPACT})


