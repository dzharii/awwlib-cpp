# Integrating `awwlib-cpp` with CMake FetchContent
Date: 2024-10-18

## Problem

When attempting to include the `awwlib-cpp` library in a client project using CMake's `FetchContent`, the following error occurred:

```
CMake Error at build/_deps/awwlib-src/CMakeLists.txt:50 (include):
  include could not find requested file:

    Misc
```

This error happened because the library's `CMakeLists.txt` used `CMAKE_SOURCE_DIR` and `PROJECT_SOURCE_DIR` to reference its own directories. However, when included via `FetchContent`, these variables point to the client's directories, not the library's. As a result, CMake couldn't locate the required files.

## Solution

To fix the issue, we updated the `CMakeLists.txt` of `awwlib-cpp` to use `CMAKE_CURRENT_SOURCE_DIR`, which always refers to the directory containing the currently processed `CMakeLists.txt`. This ensures that paths are correctly resolved whether the library is built independently or included as a subproject.

### Changes Made

**Update `CMAKE_MODULE_PATH`:**

```cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
```

**Update Include Directories:**

```cmake
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

**Review All Directory References:**

Replaced all instances of `CMAKE_SOURCE_DIR` and `PROJECT_SOURCE_DIR` intended to refer to the library's source directory with `CMAKE_CURRENT_SOURCE_DIR`.

**Change Library Type:**

Changed the library from an `OBJECT` library to a `STATIC` library to allow proper linking:

```cmake
add_library(${LIBRARY_NAME} STATIC ${SOURCES})
```

**Conditional Building for Tests and Examples:**

Adjusted the build configuration to include tests and examples only when the library is the top-level project:

```cmake
if (AWWLIB_TOPLEVEL_PROJECT)
    add_subdirectory(tests)
endif()
```

### Explanation

- **`CMAKE_CURRENT_SOURCE_DIR`**: Points to the directory where the current `CMakeLists.txt` is located, ensuring correct path resolution.

- **Library Type Change**: `OBJECT` libraries cannot be linked directly by clients. Changing to `STATIC` allows clients to link against the library.

- **Conditional Building**: Prevents unnecessary building of tests and examples when the library is used as a dependency.

## Result

After these modifications, the client project successfully included and linked the `awwlib-cpp` library using `FetchContent`, resolving the initial error.

## Summary

- **Problem**: Incorrect directory references in `CMakeLists.txt` when using `FetchContent`.

- **Solution**: Use `CMAKE_CURRENT_SOURCE_DIR` for path resolution and adjust the library type and build configurations accordingly.

- **Outcome**: The library can now be seamlessly included in client projects without errors.