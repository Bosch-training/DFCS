add_executable(harness_gtest_smoke harness/smoke_gtest.cpp)
target_link_libraries(harness_gtest_smoke PRIVATE GTest::gtest_main)
gtest_discover_tests(harness_gtest_smoke)

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
