# Code Style
Date: 2024-10-13

## Project Structure

The header and source naming conventions follow standard C++ style, which means we prefer to use `snake_case` for identifiers.

The code is organized into three main folders:

```
include/   # Header files and public interfaces
src/       # Source files and implementation code
tests/     # Unit tests and testing code
```

### Adding New Files

When adding a new file, for example, introducing a new string function, the structure will be as follows:

```
include/aww-string/aww-string.hpp
```

Here, we always prefix filenames with `aww-` to ensure internal consistency, avoid naming conflicts, and make it easy to identify project-specific files.

- `include/aww-string/` is a feature-specific folder.
- Inside `include/aww-string/`, there will also be a `README.md` file that describes all available functions. Ensure the `README.md` files are kept up-to-date as new functions are added, to maintain accurate documentation.

Other related files will be:

```
src/aww-string/aww-string.cpp     # Implementation of string functions
tests/aww-string/aww-string.cpp   # Unit tests for the string functions
```

This ensures consistency and clarity across the project.

## Code Tagging

We use `aww tag` to generate and assign unique tags to functions or code sections. The purpose of code tags is to facilitate easy referencing and locating of specific code parts, especially when using tools like GitHub code search or command-line utilities such as `grep`.

Code tags must be unique across the project. 
So, for example, in the `TODO.md` file, you could write:

> - [ ] 2024-10-13 add memoization to `fibonacci` #6a853tps1r1

The tag `#6a853tps1r1` allows the function to be easily found, even after the source file or the function has been renamed. This helps maintain traceability and simplifies searching, even with basic tools.

Always add `#` before the tag to ensure consistency. Example:

```cpp
/**
 * @brief Computes the nth Fibonacci number. #6a853tps1r1
 * 
 * This function uses a simple (very inefficient) recursive algorithm to calculate the 
 * Fibonacci number at position n.
 * 
 * @param n The position in the Fibonacci sequence (0-based).
 * @return The Fibonacci number at the given position.
 */
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}
```

By adding a unique code tag, you ensure that specific functions or sections can be referenced directly, which is especially useful for cross-referencing tasks or during refactoring.

