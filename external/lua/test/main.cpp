#include <lua.hpp>
#include <iostream>
#include <string>

// Iterative Fibonacci function in C++
int fibonacci(int n) {
    if (n <= 1) return n;
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
    int n = luaL_checkinteger(L, 1); // Get the first argument from Lua
    int result = fibonacci(n);       // Call the C++ Fibonacci function
    lua_pushinteger(L, result);      // Push the result back to Lua
    return 1;                        // Number of return values
}

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // Register the fibonacci function in Lua
    lua_register(L, "fibonacci", lua_fibonacci);

    // Load and execute the Lua script
    if (luaL_dofile(L, "printme.lua") != LUA_OK) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // Remove error message from stack
    }

    lua_close(L);
    return 0;
}
