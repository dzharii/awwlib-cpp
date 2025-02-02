cmake_minimum_required(VERSION 3.22)

# List all files containing tests. (Change as needed)
set(TESTFILES        # All .cpp files in tests/
    main.cpp
    dummy.cpp
    aww-string/aww-string-tests.cpp
    aww-os/aww-os-tests.cpp
    aww-result/aww-result-tests.cpp
    aww-collection/aww-collection-tests.cpp
    aww-value-objects/aww-value-objects-tests.cpp
    aww-pubsub/aww-pubsub-tests.cpp
    aww-configuration-value-objects/aww-configuration-value-objects-tests.cpp
    #faudv6fbgzt
)

set(TEST_MAIN unit_tests)   # Default name for test executable (change if you wish).
set(TEST_RUNNER_PARAMS "")  # Any arguemnts to feed the test runner (change as needed).

# --------------------------------------------------------------------------------
#                         Make Tests (no change needed).
# --------------------------------------------------------------------------------
add_executable(${TEST_MAIN} ${TESTFILES})
target_link_libraries(${TEST_MAIN} PRIVATE ${LIBRARY_NAME} doctest)
target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

# Suppress unused parameter warnings across different compilers in tests only.
# MSVC issues warning C4100 for unused parameters, which we disable using /wd4100.
# GCC/Clang use -Wunused-parameter, which we suppress with -Wno-unused-parameter.
if(MSVC)
    target_compile_options(${TEST_MAIN} PRIVATE /wd4100)
else()
    target_compile_options(${TEST_MAIN} PRIVATE -Wno-unused-parameter)
endif()

set_target_properties(${TEST_MAIN} PROPERTIES
    # Ensure the test executable is placed in the same directory as the main application.
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_test(
    # Use some per-module/project prefix so that it is easier to run only tests for this module
    NAME ${LIBRARY_NAME}.${TEST_MAIN}
    COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS})

# Adds a 'coverage' target.
include(CodeCoverage)
