add_library(dfcs_core STATIC
  src/core_placeholder.cpp
  src/flight_state_table.cpp
  src/flight_state_machine.cpp)
target_include_directories(dfcs_core PUBLIC include)
