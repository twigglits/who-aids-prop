cmake_minimum_required(VERSION 2.8)

project(simpact-cyan)
set(VERSION 0.2.0)
include(${PROJECT_SOURCE_DIR}/cmake/SimpactMacros.cmake)

simpact_setup()

# This contains the main simpact program
add_subdirectory(src)

