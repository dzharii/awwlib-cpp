#include "doctest/doctest.h"
#include "aww-string/aww-string.hpp"

TEST_CASE("string_trim_inplace") {
    SUBCASE("Trim leading and trailing spaces") {
        std::string str = "   Hello, World!   ";
        aww::string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("Trim leading spaces") {
        std::string str = "   Hello, World!";
        aww::string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("Trim trailing spaces") {
        std::string str = "Hello, World!   ";
        aww::string_trim_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("No spaces") {
        std::string str = "Hello,World!";
        aww::string_trim_inplace(str);
        CHECK(str == "Hello,World!");
    }

    SUBCASE("Empty string") {
        std::string str = "";
        aww::string_trim_inplace(str);
        CHECK(str == "");
    }
}


TEST_CASE("string_trim_right_inplace") {
    SUBCASE("Trim trailing spaces") {
        std::string str = "Hello, World!   ";
        aww::string_trim_right_inplace(str);
        CHECK(str == "Hello, World!");
    }

    SUBCASE("No spaces") {
        std::string str = "Hello,World!";
        aww::string_trim_right_inplace(str);
        CHECK(str == "Hello,World!");
    }

    SUBCASE("Empty string") {
        std::string str = "";
        aww::string_trim_right_inplace(str);
        CHECK(str == "");
    }
}

TEST_CASE("aww::join_vector returns joined string (aww tag #oibw1sg0jpv)") {
    CHECK(aww::join_vector({"hello", "world"}, " ") == "hello world");
    CHECK(aww::join_vector({"hello", "world"}, " - ") == "hello - world");
    CHECK(aww::join_vector({"hello"}, " - ") == "hello");
    CHECK(aww::join_vector({}, " - ") == "");
}
