add_library(dfcs_core STATIC src/core_placeholder.cpp src/pid.cpp)
target_include_directories(dfcs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
