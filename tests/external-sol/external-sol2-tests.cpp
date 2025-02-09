#include "doctest/doctest.h"
#include <sol/sol.hpp>
#include <iostream>

TEST_CASE("Sol2 Lua Integration") {

    // Create a new Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    SUBCASE("Basic Lua Execution") {
        lua.script("x = 42");
        int x = lua["x"];
        CHECK(x == 42);  // Ensure Lua variable assignment works
    }

    SUBCASE("Calling Lua Functions from C++") {
        lua.script(R"(
            function add(a, b)
                return a + b
            end
        )");

        sol::function add = lua["add"];
        int result = add(3, 7);
        CHECK(result == 10);  // Verify function result
    }

    SUBCASE("Calling C++ Functions from Lua") {
        lua.set_function("cpp_function", []() { return 99; });

        lua.script("value = cpp_function()");
        int value = lua["value"];
        CHECK(value == 99);  // Ensure Lua can call C++ functions
    }

    SUBCASE("Handling Tables") {
        lua.script(R"(
            person = {
                name = "Alice",
                age = 25
            }
        )");

        sol::table person = lua["person"];
        std::string name = person["name"];
        int age = person["age"];

        CHECK(name == "Alice");
        CHECK(age == 25);
    }
}
