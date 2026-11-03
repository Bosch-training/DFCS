add_executable(harness_gtest_smoke harness/smoke_gtest.cpp)
target_link_libraries(harness_gtest_smoke PRIVATE GTest::gtest_main)
gtest_discover_tests(harness_gtest_smoke)

# DC-50 (CTL-01): Pid unit under test in dfcs_core, GoogleTest, no fakes (design 4.1).
add_executable(dfcs_ctrl_pid_tests ctrl/pid_gtest.cpp)
target_include_directories(dfcs_ctrl_pid_tests
  PRIVATE "${PROJECT_SOURCE_DIR}/firmware/core/src")
target_link_libraries(dfcs_ctrl_pid_tests PRIVATE dfcs_core GTest::gtest_main)
gtest_discover_tests(dfcs_ctrl_pid_tests)

add_executable(harness_unity_smoke harness/smoke_unity.c)
target_link_libraries(harness_unity_smoke PRIVATE unity)
add_test(NAME harness_unity_smoke COMMAND harness_unity_smoke)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -m pytest --version
  RESULT_VARIABLE dfcs_pytest_rc OUTPUT_QUIET ERROR_QUIET)
if(NOT dfcs_pytest_rc EQUAL 0)
  message(FATAL_ERROR
    "pytest not found for ${Python3_EXECUTABLE}; pass -DPython3_EXECUTABLE=<python with pytest>")
endif()
add_test(
  NAME harness_pytest_smoke
  COMMAND "${Python3_EXECUTABLE}" -m pytest -q "${CMAKE_CURRENT_SOURCE_DIR}/python"
  )
