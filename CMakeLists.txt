cmake_minimum_required(VERSION 3.25)

project(dfcs
  VERSION 0.1.0
  DESCRIPTION "Drone Flight Control System (POC, not flight-ready, NFR-017)"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Portable flight-control core, buildable for the desktop host (FR-070).
add_subdirectory(firmware/core)
