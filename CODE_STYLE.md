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