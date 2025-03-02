#include "aww-string/aww-string.hpp"
#include "doctest/doctest.h"

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

TEST_CASE("to_lower_case") {
  CHECK(aww::to_lower_case("Hello, World!") == "hello, world!");
  CHECK(aww::to_lower_case("HELLO") == "hello");
  CHECK(aww::to_lower_case("hello") == "hello");
  CHECK(aww::to_lower_case("") == "");
}

TEST_CASE("to_lower_case_inplace") {
  std::string str1 = "Hello, World!";
  aww::to_lower_case_inplace(str1);
  CHECK(str1 == "hello, world!");

  std::string str2 = "HELLO";
  aww::to_lower_case_inplace(str2);
  CHECK(str2 == "hello");

  std::string str3 = "hello";
  aww::to_lower_case_inplace(str3);
  CHECK(str3 == "hello");

  std::string str4 = "";
  aww::to_lower_case_inplace(str4);
  CHECK(str4 == "");
}

TEST_CASE("string_remove_all_whitespaces - Empty String") {
  std::string input = "";
  std::string expected = "";
  CHECK(aww::string_remove_all_whitespaces(input) == expected);
}

TEST_CASE("string_remove_all_whitespaces - No Whitespace Present") {
  std::string input = "HelloWorld!";
  std::string expected = "HelloWorld!";
  CHECK(aww::string_remove_all_whitespaces(input) == expected);
}

TEST_CASE("string_remove_all_whitespaces - String with Only Spaces") {
  std::string input = "   ";
  std::string expected = "";
  CHECK(aww::string_remove_all_whitespaces(input) == expected);
}

TEST_CASE("string_remove_all_whitespaces - Mixed Whitespace Characters") {
  std::string input = "  Hello\tWorld \nTest\nCase  ";
  std::string expected = "HelloWorldTestCase";
  CHECK(aww::string_remove_all_whitespaces(input) == expected);
}

TEST_CASE("string_remove_all_whitespaces - Complex Sentence") {
  std::string input = "   The quick brown fox  jumps over\t the lazy dog.\n";
  std::string expected = "Thequickbrownfoxjumpsoverthelazydog.";
  CHECK(aww::string_remove_all_whitespaces(input) == expected);
}
