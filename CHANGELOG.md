# AwwLib Changelog



## 2025-02-02

Added the `aww-configuration-value-objects` module to support common configuration value objects. This module introduces the `hex_color` class (with optional alpha support) along with its derivatives `background_color` and `foreground_color` for consistent handling of hexadecimal color values.

```cpp
// Use the background_color class.
aww::background_color bg_color("#ABCDEF");
std::cout << "Background color:\n";
std::cout << "  red:   " << bg_color.red() << "\n";
std::cout << "  green: " << bg_color.green() << "\n";
std::cout << "  blue:  " << bg_color.blue() << "\n\n";
```



## 2025-01-30

Added Publisher Subscriber class! 

```cpp
#include <iostream>
#include "aww-pubsub/aww-pubsub.hpp"

struct foo_event {
  int value;
};

struct bar_event {
  std::string name;
};

int main() {
  aww::pubsub_event_bus bus;

  auto foo_sub = bus.subscribe<foo_event>([](const foo_event& foo) {
    std::cout << "foo_event -> value=" << foo.value << "\n";
  });

  auto bar_sub = bus.subscribe<bar_event>([](const bar_event& bar) {
    std::cout << "bar_event -> name=" << bar.name << "\n";
  });

  bus.publish(foo_event{123});
  bus.publish(bar_event{"HelloWorld"});

  return 0;
}

```



## 2025-01-22 

Refactoring.

**`result_error` Struct Simplification**:
The `result_error` struct was updated to remove the `error_code` field. This revision narrows its scope to handling descriptive error messages, making the structure more concise and specialized.

**Test File Naming Update**:
A minor change was made to the `Generate-Files.ps1` script to rename generated test files. Test files now include the `-tests` suffix (e.g., `example-tests.cpp`) for clearer identification.

`awwy gen` now creates test files with `-tests` prefix; example: `aww-string-tests.cpp`



aww::reuslt::err now has simplified API simplified, which allows creating error from `std::string` and `std::exception`:

```cpp
SUBCASE("Create and access error with std::string") {
  aww::result<int> res = aww::result<int>::err(std::string("String Error"));
  CHECK(res.is_ok() == false);
  CHECK(res.is_err() == true);
  CHECK(static_cast<bool>(res) == false);
  CHECK(res.error().error_message() == "String Error");
}

SUBCASE("Create and access error with std::exception") {
  try {
    throw std::runtime_error("Exception Error");
  } catch (const std::exception& e) {
    aww::result<int> res = aww::result<int>::err(e);
    CHECK(res.is_ok() == false);
    CHECK(res.is_err() == true);
    CHECK(static_cast<bool>(res) == false);
    CHECK(res.error().error_message() == "Exception Error");
  }
}
```





## 2025-01-18

**Value Objects** in `include\aww-value-objects\aww-value-objects.hpp`

**Usage examples**: `tests\aww-value-objects\aww-value-objects.cpp`

**What's the Idea?**

I’ve added support for **value objects** to improve how we handle configuration values. Value objects are immutable and validate themselves when created, ensuring they’re always in a valid state. This means we no longer have to scatter validation logic across the codebase—it’s all baked into the object itself.

This approach simplifies the code and avoids repetitive checks like ensuring numbers are positive or strings aren’t empty. It also makes these values thread-safe since they can’t be changed after creation.

**Example in Action**

Here’s a simple example:

```cpp
class NonNegativeInt : public ValueObject<int> {
public:
    explicit NonNegativeInt(int value) : ValueObject(value) {
        if (value < 0) throw std::invalid_argument("Value must be non-negative");
    }
};

// Usage
NonNegativeInt positiveValue(42); // Works fine
NonNegativeInt invalidValue(-1); // Throws an exception
```

**Why It’s Useful**

This keeps the code clean and ensures every value is valid at creation. No surprises later and no redundant validation checks scattered around. Plus, immutability makes these values safe to copy and use anywhere. Check out the test file for more examples!



## 2025-01-10

Enhanced cross-platform CLI argument handling has been introduced with the addition of the `get_command_line_arguments` function in both Linux and Windows implementations, allowing for retrieval of command-line arguments while excluding the program name. On Windows, UTF-16 to UTF-8 conversion has been implemented to ensure robust argument parsing. This update also includes the addition of the `<vector>` header in `aww-os.hpp` to support the new functionality, along with minor codebase improvements such as resolving redundant namespace closing syntax in `aww-collection.hpp`. The changes are tagged with the identifier **#w9e1s4uq1xt** for tracking.



- [x] 🚩TODO: move `utf16_to_utf8` to aww-string
  - [x] Will keep it there, since it is for windows
- [x] 🚩TODO: I guess I can add tests for `get_command_line_arguments` aww tag #w9e1s4uq1xt only on Linux, since on windows it uses winapi call inside
  - [x] Also no, since it has a windows call



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
