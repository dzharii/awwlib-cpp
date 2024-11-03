# This file specifies how the project should be built, using CMake.
# If you are unfamiliar with CMake, don't worry about all the details.
# The sections you might want to edit are marked as such, and
# the comments should hopefully make most of it clear.
#
# For many purposes, you may not need to change anything about this file.

cmake_minimum_required(VERSION 3.22)

# Set project name, version and laguages here. (change as needed)
# Version numbers are available by including "exampleConfig.h" in
# the source. See exampleConfig.h.in for some more details.
project(AWWLIB VERSION 1.0.0 LANGUAGES CXX)

# Determine if we are the top-level project
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(AWWLIB_TOPLEVEL_PROJECT TRUE)
else()
    set(AWWLIB_TOPLEVEL_PROJECT FALSE)
endif()

# This magic allows to build Debug and Release configuration in separate
# directories for Windows / Linux
set(MY_BUILD_OUTPUT_DIRECTORY "")
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()


# Options: Things you can set via commandline options to cmake (e.g. -DENABLE_LTO=[ON|OFF])
option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)

option(AWWLIB_BUILD_TESTS "Build the tests" ${AWWLIB_TOPLEVEL_PROJECT})
option(AWWLIB_BUILD_EXAMPLES "Build the examples" ${AWWLIB_TOPLEVEL_PROJECT})

# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(CTest)
include(Doctest)
include(LTO)
include(Misc)
include(Warnings)

# Check for LTO support.
find_lto(CXX)

# --------------------------------------------------------------------------------
#                         Locate files (change as needed).
# --------------------------------------------------------------------------------
set(SOURCES          # All .cpp files in src/
    src/example.cpp
    src/aww-string/aww-string.cpp
    src/aww-os/aww-os.cpp
    src/aww-result/aww-result.cpp
    src/aww-collection/aww-collection.cpp
    #ge0mh0v43gk
)

if(WIN32)
    list(APPEND SOURCES
        src/aww-os/aww-os-windows.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCES
        src/aww-os/aww-os-linux.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(TESTFILES        # All .cpp files in tests/
    #5x0qd25zn50
)
set(LIBRARY_NAME awwlib)  # Default name for the library built from src/*.cpp (change if you wish)

# --------------------------------------------------------------------------------
#                            Build! (Change as needed)
# --------------------------------------------------------------------------------
# Compile all sources into a library.
add_library(${LIBRARY_NAME} STATIC ${SOURCES})

# Lib needs its header files, and users of the library must also see these (PUBLIC). (No change needed)
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# There's also (probably) doctests within the library, so we need to see this as well.
target_link_libraries(${LIBRARY_NAME} PUBLIC doctest)

# Set the compile options you want (change as needed).
target_set_warnings(${LIBRARY_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
# target_compile_options(${LIBRARY_NAME} ... )  # For setting manually.

# Add an executable for the file app/main.cpp.
# If you add more executables, copy these lines accordingly.
if (AWWLIB_BUILD_EXAMPLES)
    add_executable(main app/main.cpp)
    target_link_libraries(main PRIVATE ${LIBRARY_NAME})
    target_set_warnings(main ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
    target_enable_lto(main optimized)  # enable link-time-optimization if available for non-debug configurations
    # Set the properties you require, e.g. what C++ standard to use. Here applied to library and main (change as needed).
    set_target_properties(
          main
          PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
    )
endif()
set_target_properties(
    ${LIBRARY_NAME}
    PROPERTIES
      CXX_STANDARD 20
      CXX_STANDARD_REQUIRED YES
      CXX_EXTENSIONS NO
)

if (AWWLIB_BUILD_TESTS)
    # Set up tests (see tests/CMakeLists.txt).
    add_subdirectory(tests)
endif()
