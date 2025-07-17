// File: tests/aww-special-string/aww-special-string-tests.cpp
#include "aww-special-string/aww-special-string.hpp"
#include "doctest/doctest.h"
#include <string>
#include <string_view>

using aww::safe_filename_from_string;

// helper lambdas for UTF-8 char8_t string literals
static const auto u8sv = [](const char8_t* lit) -> std::string_view {
  return {reinterpret_cast<const char*>(lit), std::char_traits<char8_t>::length(lit)};
};
static const auto u8s = [](const char8_t* lit) -> std::string {
  return {reinterpret_cast<const char*>(lit), std::char_traits<char8_t>::length(lit)};
};

TEST_CASE("safe_filename_from_string handles typical and edge cases") {
  SUBCASE("Empty input yields empty output") {
    CHECK(safe_filename_from_string("", '_').empty());
  }

  SUBCASE("Already safe name stays unchanged") {
    CHECK_EQ(safe_filename_from_string("report_2025.txt", '_'), "report_2025.txt");
  }

  SUBCASE("Only unsafe characters collapse to empty output") {
    CHECK(safe_filename_from_string(":::////****", '_').empty());
  }

  SUBCASE("Consecutive unsafe runs fold into one underscore") {
    CHECK_EQ(safe_filename_from_string("file///:??name", '_'), "file_name");
  }

  SUBCASE("Trailing dots are removed") {
    CHECK_EQ(safe_filename_from_string("logfile.", '_'), "logfile");
  }

  SUBCASE("Trailing spaces are removed") {
    CHECK_EQ(safe_filename_from_string("logfile   ", '_'), "logfile");
  }

  SUBCASE("Control characters are treated as unsafe") {
    std::string with_ctrl = std::string("file") + char(0x1F) + "name";
    CHECK_EQ(safe_filename_from_string(with_ctrl, '_'), "file_name");
  }

  SUBCASE("Russian input: colon replaced with underscore") {
    CHECK_EQ(safe_filename_from_string(u8sv(u8"пример:файл.txt"), '_'), u8s(u8"пример_файл.txt"));
  }

  SUBCASE("Japanese input: slash replaced with underscore") {
    CHECK_EQ(safe_filename_from_string(u8sv(u8"データ/バックアップ"), '_'), u8s(u8"データ_バックアップ"));
  }

  SUBCASE("Input consisting solely of slashes yields empty output") {
    CHECK(safe_filename_from_string("////", '_').empty());
  }

  SUBCASE("Input consisting of single replacement character yields empty output") {
    CHECK(safe_filename_from_string("_", '_').empty());
  }

  SUBCASE("Input consisting of single replacement character with different replacement yields that character") {
    CHECK_EQ(safe_filename_from_string("_", '-'), "_");
  }

  SUBCASE("Result that would be single replacement character yields empty output") {
    CHECK(safe_filename_from_string("///", '_').empty());
  }

  SUBCASE("Mixed content with different replacement character") {
    CHECK_EQ(safe_filename_from_string("file///:??name", '-'), "file-name");
  }

  SUBCASE("Replacement character appears in safe content") {
    CHECK_EQ(safe_filename_from_string("file_name", '_'), "file_name");
  }
}
