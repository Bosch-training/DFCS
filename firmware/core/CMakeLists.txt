add_library(dfcs_core STATIC
  src/core_placeholder.cpp
  src/flight_state_table.cpp
  src/flight_state_machine.cpp
  src/pid.cpp)
target_include_directories(dfcs_core PUBLIC include "${CMAKE_CURRENT_SOURCE_DIR}/src")
