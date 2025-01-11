## 2025-01-10

Enhanced cross-platform CLI argument handling has been introduced with the addition of the `get_command_line_arguments` function in both Linux and Windows implementations, allowing for retrieval of command-line arguments while excluding the program name. On Windows, UTF-16 to UTF-8 conversion has been implemented to ensure robust argument parsing. This update also includes the addition of the `<vector>` header in `aww-os.hpp` to support the new functionality, along with minor codebase improvements such as resolving redundant namespace closing syntax in `aww-collection.hpp`. The changes are tagged with the identifier **#w9e1s4uq1xt** for tracking.



## 2024-11-09 

**Lua integration!** 

Thank to the project: 

2024-11-09 [marovira/lua: The Lua Programming Language with Modern CMake](https://github.com/marovira/lua/tree/master) { github.com }

Lua is published under the MIT license and can be viewed [here](https://github.com/marovira/lua/blob/master/LUA_LICENSE). For more information, please see their official website [here](https://www.lua.org/).

This bundle is published under the BSD-3 license can be viewed [here](https://github.com/marovira/lua/blob/master/LICENSE)



## 2024-11-03

tiny cleanup  -- remove `pragma once`

add `erase_all_elements` this will be first integration with `awwtools` project

this is the first integration with `awwlib`

## 2024-10-25

Changelog started!

**New Features:**

- Introduction of the `aww::result` class for enhanced error handling and result management.
- Addition of PowerShell scripts to streamline Git workflows and change management.

**Configuration Enhancements:**

- Establishment of `.editorconfig` for consistent code styling.
- Initialization of `CHANGELOG.md` for ongoing project documentation.

**Testing Improvements:**

- Comprehensive unit tests for the new `aww::result` module to ensure functionality and reliability.
