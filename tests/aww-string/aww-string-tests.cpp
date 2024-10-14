#include "doctest/doctest.h"
#include "aww-string/aww-string.hpp"

TEST_CASE("string_trim_inplace") {
    using namespace aww;

    SUBCASE("Trim leading and trailing spaces") {
        std::string str = "   Hello, World!   ";
        string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("Trim leading spaces") {
        std::string str = "   Hello, World!";
        string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("Trim trailing spaces") {
        std::string str = "Hello, World!   ";
        string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("No spaces") {
        std::string str = "Hello,World!";
        string_trim_inplace(str);
        CHECK(str == "Hello,World!");
    }

    SUBCASE("Empty string") {
        std::string str = "";
        string_trim_inplace(str);
        CHECK(str == "");
    }
}
