#include "aww-collection/aww-collection.hpp"
#include "doctest/doctest.h"
#include <string>
#include <vector>

TEST_CASE("erase_all_elements") {
  SUBCASE("removes all occurrences of a specified value from a vector of strings") {
    std::vector<std::string> vec = {"--verbose", "--help", "--verbose"};
    bool removed = aww::erase_all_matched_elements(vec, std::string("--verbose"));
    CHECK(removed == true);
    CHECK(vec == std::vector<std::string>{"--help"});
  }

  SUBCASE("returns false when the value is not found in the vector") {
    std::vector<std::string> vec = {"--help", "--version"};
    bool removed = aww::erase_all_matched_elements(vec, std::string("--verbose"));
    CHECK(removed == false);
    CHECK(vec == std::vector<std::string>{"--help", "--version"});
  }

  SUBCASE("removes all occurrences of a specified value from a vector of integers") {
    std::vector<int> vec = {1, 2, 3, 2, 4};
    bool removed = aww::erase_all_matched_elements(vec, 2);
    CHECK(removed == true);
    CHECK(vec == std::vector<int>{1, 3, 4});
  }

  SUBCASE("returns false when the vector is empty") {
    std::vector<int> vec;
    bool removed = aww::erase_all_matched_elements(vec, 1);
    CHECK(removed == false);
    CHECK(vec.empty() == true);
  }

  SUBCASE("removes all occurrences of a specified value from a vector of custom objects") {
    struct Custom {
      int id;
      bool operator==(const Custom& other) const { return id == other.id; }
    };

    std::vector<Custom> vec = {{1}, {2}, {3}, {2}, {4}};
    bool removed = aww::erase_all_matched_elements(vec, Custom{2});
    CHECK(removed == true);
    CHECK(vec == std::vector<Custom>{{1}, {3}, {4}});
  }
}
