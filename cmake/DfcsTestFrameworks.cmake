# Fetch test frameworks at the exact commits pinned in tools/toolchain.json (NFR-010).
include(FetchContent)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../tools/toolchain.json" DFCS_TOOLCHAIN_JSON)

function(dfcs_declare_pinned name)
  string(JSON repo GET "${DFCS_TOOLCHAIN_JSON}" libraries ${name} repository)
  string(JSON commit GET "${DFCS_TOOLCHAIN_JSON}" libraries ${name} commit)
  FetchContent_Declare(${name} GIT_REPOSITORY ${repo} GIT_TAG ${commit} GIT_SHALLOW OFF)
endfunction()

dfcs_declare_pinned(googletest)
dfcs_declare_pinned(unity)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest unity)
