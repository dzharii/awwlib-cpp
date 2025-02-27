# AwwLib Changelog



## 2025-02-27 Late night libtidy!

Included libtidy!

2025-02-27 [Tidy Developer](https://www.html-tidy.org/developer/) {www.html-tidy.org}

- 2025-02-27 [Tidy - Browse /5.8.0 at SourceForge.net](https://sourceforge.net/projects/html-tidy.mirror/files/5.8.0/) {sourceforge.net}



## 2025-02-26 

Continue with `aww::sanitize_html`; I have found this amazing bag of XSSes:

2025-02-26 🚩 [payloadbox/xss-payload-list: 🎯 Cross Site Scripting ( XSS ) Vulnerability Payload List](https://github.com/payloadbox/xss-payload-list) { github.com }



Also, once the tests are fixed, it is time to extend the allowed tag list:

> ✅ **Recommended Safe Text Elements**
>
> #### **Text Content and Inline Formatting**
>
> - `<b>` – Bold text (without emphasis)
> - `<strong>` – Important text (semantic emphasis)
> - `<i>` – Italicized text (without emphasis)
> - `<em>` – Emphasized text (semantic emphasis)
> - `<u>` – Underlined text
> - `<s>` – Strikethrough text
> - `<sub>` – Subscript text
> - `<sup>` – Superscript text
> - `<small>` – Smaller text
> - `<mark>` – Highlighted text
> - `<abbr>` – Abbreviations (with title attributes for explanations)
> - `<cite>` – Citation of works
> - `<q>` – Inline quotations
> - `<code>` – Inline code snippets
> - `<kbd>` – Keyboard input representation
> - `<var>` – Variable names in programming/math context
> - `<time>` – Machine-readable dates/times
> - `<dfn>` – Definition term
> - `<bdi>` – Bi-directional isolation
> - `<bdo>` – Bi-directional override
>
> ------
>
> **Block-Level Text Content Elements**
>
> - `<p>` – Paragraphs
> - `<blockquote>` – Block quotations
> - `<pre>` – Preformatted text (for code blocks or ASCII art)
> - `<h1>`, `<h2>`, `<h3>`, `<h4>`, `<h5>`, `<h6>` – Headings
> - `<hr>` – Horizontal rule (for thematic breaks)
> - `<br>` – Line breaks
>
> ------
>
> **Lists (Textual Only)**
>
> - `<ul>` – Unordered lists
> - `<ol>` – Ordered lists
> - `<li>` – List items
> - `<dl>` – Description lists
> - `<dt>` – Description terms
> - `<dd>` – Description definitions
>
> ------
>
> **Text-Related Links (Optional for Text Context)**
>
> - `<a>` – Hyperlinks (only if you sanitize `href` attributes properly)
>
> ------
>
> 🚫 **Elements to Exclude for Text-Only Sanitization**
>
> - `<div>`, `<span>` – Purely structural/neutral (unless you need attributes like `lang`, which you can whitelist separately)
> - `<table>`, `<tr>`, `<td>`, `<th>` – Structural, not for plain text
> - `<form>`, `<input>`, `<button>`, `<select>` – Interactive elements
> - `<script>`, `<style>`, `<iframe>` – Security risks, always exclude
> - `<img>`, `<video>`, `<audio>` – Media, not textual
> - `<nav>`, `<aside>`, `<section>`, `<article>`, `<header>`, `<footer>` – Structural content
> - `<figure>`, `<figcaption>` – Media captions (exclude unless captions are critical)
> - `<canvas>`, `<svg>`, `<object>` – Non-text visual elements
>
> ------
>
> 🔒 **Security Considerations**
>
> - **Attributes**: Strip all attributes except safe ones (e.g., `href` in `<a>`, `title` in `<abbr>`, `datetime` in `<time>`).
> - **Event Handlers**: Remove all `on*` event attributes (like `onclick`) to prevent XSS attacks.
> - **Protocol Whitelisting**: For `<a href>`, allow only safe protocols (`http`, `https`, `mailto`), block `javascript:`, `data:`, etc.





## 2025-02-25

Continue with `aww::sanitize_html`

I have discovered this project, which I am planning to use for testing and coding inspiration (yep, by rewriting some code)

**Credit:**

This set of tests was created by **Michael Ganss**.

- **Author Profile:** [Michael Ganss on GitHub](https://github.com/mganss)
- **Project:** [HtmlSanitizer on GitHub](https://github.com/mganss/HtmlSanitizer)
  *HtmlSanitizer* is a .NET library that cleans HTML to prevent XSS attacks by removing unwanted tags, attributes, and scripts while allowing safe elements and formatting.





## 2025-02-25

I have added a new HTML sanitizer in our project (see `include/aww-html/aww-html.hpp`). The sanitizer now only allows safe tags (like `<h1>`–`<h6>`, `<p>`, `<b>`, `<i>`, `<em>`, `<strong>`, and `<a>`) and strips out everything else. For `<a>` tags, it keeps only the `href` attribute if it starts with "http://" or "https://", blocking unsafe URL schemes. It also auto-closes unclosed tags and detects obfuscated markup to prevent hidden injections. On top of that, I’ve added a feature to strip all HTML comments, so no hidden code remains.

For example:

```cpp
std::string input = "<h1>Welcome</h1><p>Hello <!-- secret code --> World</p>";
auto result = aww::sanitize_html(input);
// result.value() returns: "<h1>Welcome</h1><p>Hello World</p>"
```

This update improves our security against XSS and other injection attacks.

```cpp
xss("<script>alert('XSS')</script>") == sanitized ==> ""
xss("<img src=x onerror=alert('XSS')>") == sanitized ==> "&lt;img src=x onerror=alert('XSS')"
xss("<a href=\"javascript:alert(1)\">Click me</a>") == sanitized ==> "<a>Click me</a>"
xss("<scr<script>ipt>alert('XSS')</scr<script>ipt>") == sanitized ==> "alert('XSS')"
xss("<!-- <script>alert('XSS')</script> -->") == sanitized ==> ""
xss("<svg/onload=alert('XSS')>") == sanitized ==> "alert('XSS')"
xss("<a href='data:text/html;base64,PHNjcmlwdD5hbGVydCgxKTwvc2NyaXB0Pg=='>Test</a>") == sanitized ==> "<a>Test</a>"

```



## 2025-02-11

Added utf8 library

`external\utf8proc-2.10.0`



2025-02-12 [JuliaStrings/utf8proc: a clean C library for processing UTF-8 Unicode data](https://github.com/JuliaStrings/utf8proc) { github.com }

> [juliastrings.github.io/utf8proc/](http://juliastrings.github.io/utf8proc/)
>
> [utf8proc](http://juliastrings.github.io/utf8proc/) is a small, clean C library that provides Unicode normalization, case-folding, and other operations for data in the [UTF-8 encoding](http://en.wikipedia.org/wiki/UTF-8). It was [initially developed](http://www.public-software-group.org/utf8proc) by Jan Behrens and the rest of the [Public Software Group](http://www.public-software-group.org/), who deserve *nearly all of the credit* for this package. With the blessing of the Public Software Group, the [Julia developers](http://julialang.org/) have taken over development of utf8proc, since the original developers have moved to other projects.
>
> (utf8proc is used for basic Unicode support in the [Julia language](http://julialang.org/), and the Julia developers became involved because they wanted to add Unicode 7 support and other features.)
>
> (The original utf8proc package also includes Ruby and PostgreSQL plug-ins. We removed those from utf8proc in order to focus exclusively on the C library.)
>
> The utf8proc package is licensed under the free/open-source [MIT "expat" license](http://opensource.org/licenses/MIT) (plus certain Unicode data governed by the similarly permissive [Unicode data license](http://www.unicode.org/copyright.html#Exhibit1)); please see the included `LICENSE.md` file for more detailed information.

And supporting acceptance tests at `tests/external-utf8proc/external-utf8proc-tests.cpp`



## 2025-02-08

I have watched Jason Turner's talk:

[Secrets of C++ Scripting Bindings: Bridging Compile Time and Run](https://www.youtube.com/watch?v=Ny9-516Gh28) { www.youtube.com }

and decided to integrate Sol2 to support better binding with Lua [ThePhD/sol2: Sol3 (sol2 v3.0)](https://github.com/ThePhD/sol2) {github.com}

> C++ <-> Lua API wrapper with advanced features and top notch performance - is here, and it's great!
>
> [sol2.rtfd.io/](http://sol2.rtfd.io/)

```cpp
#include <sol/sol.hpp>
#include <iostream>

int main() {
    // Create a new Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // Define a simple Lua script
    const std::string script = R"(
        function greet(name)
            print("Hello, " .. name .. "!")
        end

        function add(a, b)
            return a + b
        end
    )";

    // Execute the Lua script
    lua.script(script);

    // Call Lua function from C++
    lua["greet"]("World");

    // Get Lua function and call it
    sol::function add = lua["add"];
    int result = add(3, 4);
    std::cout << "Result of add(3, 4): " << result << std::endl;

    return 0;
}
```

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

tiny cleanup -- remove `pragma once`

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
