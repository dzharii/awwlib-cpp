// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#endif

#include "example.h"
#include "exampleConfig.h"
#include <cstdlib>
#include <iostream>
#include <lua.hpp>
#include <string>

// Iterative Fibonacci function in C++
int fibonacci(int n) {
  if (n <= 1)
    return n;
  int a = 0, b = 1, c;
  for (int i = 2; i <= n; ++i) {
    c = a + b;
    a = b;
    b = c;
  }
  return b;
}

// Wrapper function to expose C++ function to Lua
int lua_fibonacci(lua_State* L) {
  int n = static_cast<int>(luaL_checkinteger(L, 1));    // Safely convert lua_Integer to int
  int result = fibonacci(n);                            // Call the C++ Fibonacci function
  lua_pushinteger(L, static_cast<lua_Integer>(result)); // Safely convert int to lua_Integer
  return 1;                                             // Number of return values
}

/*
 * Simple main program that demonstrates how to access
 * CMake definitions (here the version number) from source code.
 */
int main() {
  std::cout << "C++ Boiler Plate v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << "."
            << PROJECT_VERSION_PATCH << std::endl;

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  // Register the C++ fibonacci function in Lua
  lua_register(L, "fibonacci", lua_fibonacci);

  // Lua script to calculate Fibonacci using the registered C++ function
  const char* lua_script = R"(
        -- Lua script to calculate Fibonacci
        local n = 10
        print("Calculating Fibonacci(" .. n .. ") using C++ function:")
        local result = fibonacci(n)
        print("Result:", result)
    )";

  // Load and execute the Lua script from the string
  if (luaL_dostring(L, lua_script) != LUA_OK) {
    std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1); // Remove error message from stack
  }

  lua_close(L);

  // Display LICENSE file content (non-essential for most builds)
  [[maybe_unused]] int res = std::system("cat ../LICENSE");

  // Bring in the dummy class from the example source,
  // just to show that it is accessible from main.cpp.
  Dummy d;
  return d.doSomething() ? 0 : -1;
}
