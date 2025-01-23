#include "aww-result/aww-result.hpp"
#include "doctest/doctest.h"

#include <string>
#include <utility>

// A custom type for testing
struct CustomType {
  int id;
  std::string name;

  bool operator==(const CustomType& other) const {
    return id == other.id && name == other.name;
  }
};

// Helper function to create a default error
aww::result_error make_default_error() {
  return aww::result_error("Not Found");
}

TEST_SUITE("aww::result Tests") {

  // ----------------------------
  // Success Result Tests
  // ----------------------------
  TEST_CASE("Success Result Creation and Access") {
    SUBCASE("Create and access int value") {
      aww::result<int> res = aww::result<int>::ok(42);
      CHECK(res.is_ok() == true);
      CHECK(res.is_err() == false);
      CHECK(static_cast<bool>(res) == true);
      CHECK(res.value() == 42);
    }

    SUBCASE("Create and access std::string value") {
      std::string test_str = "Hello, World!";
      aww::result<std::string> res = aww::result<std::string>::ok(test_str);
      CHECK(res.is_ok() == true);
      CHECK(res.is_err() == false);
      CHECK(static_cast<bool>(res) == true);
      CHECK(res.value() == test_str);
    }

    SUBCASE("Create and access CustomType value") {
      CustomType ct{1, "Test"};
      aww::result<CustomType> res = aww::result<CustomType>::ok(ct);
      CHECK(res.is_ok() == true);
      CHECK(res.is_err() == false);
      CHECK(static_cast<bool>(res) == true);
      CHECK(res.value() == ct);
    }
  }

  // ----------------------------
  // Error Result Tests
  // ----------------------------
  TEST_CASE("Error Result Creation and Access") {
    SUBCASE("Create and access error") {
      aww::result<int> res = aww::result<int>::err(aww::result_error("Invalid Input"));
      CHECK(res.is_ok() == false);
      CHECK(res.is_err() == true);
      CHECK(static_cast<bool>(res) == false);
      CHECK(res.error().error_message() == "Invalid Input");
    }

    SUBCASE("Create and access default error") {
      aww::result<std::string> res = aww::result<std::string>::err(make_default_error());
      CHECK(res.is_ok() == false);
      CHECK(res.is_err() == true);
      CHECK(static_cast<bool>(res) == false);
      CHECK(res.error().error_message() == "Not Found");
    }
  }

  // ----------------------------
  // Exception Handling Tests
  // ----------------------------
  TEST_CASE("Exception Handling on Invalid Access") {
    SUBCASE("Access value from error result throws") {
      aww::result<int> res = aww::result<int>::err(aww::result_error("Operation Failed"));
      CHECK_THROWS_AS(res.value(), std::runtime_error);
      CHECK_THROWS_WITH(res.value(), "Attempted to get value from an error result");
    }

    SUBCASE("Access error from success result throws") {
      aww::result<int> res = aww::result<int>::ok(7);
      CHECK_THROWS_AS(res.error(), std::runtime_error);
      CHECK_THROWS_WITH(res.error(), "Attempted to get error from a success result");
    }
  }

  // ----------------------------
  // Move Semantics Tests
  // ----------------------------
  TEST_CASE("Move Semantics") {
    SUBCASE("Move a success result") {
      aww::result<std::string> res1 = aww::result<std::string>::ok("Move Test");
      aww::result<std::string> res2 = std::move(res1);
      CHECK(res2.is_ok() == true);
      CHECK(res2.value() == "Move Test");
    }

    SUBCASE("Move an error result") {
      aww::result<int> res1 = aww::result<int>::err(aww::result_error("Move Error"));
      aww::result<int> res2 = std::move(res1);
      CHECK(res2.is_err() == true);
      CHECK(res2.error().error_message() == "Move Error");
    }
  }

  // ----------------------------
  // Copy Semantics Tests
  // ----------------------------
  TEST_CASE("Copy Semantics") {
    SUBCASE("Copy a success result") {
      aww::result<int> res1 = aww::result<int>::ok(55);
      aww::result<int> res2 = res1;
      CHECK(res2.is_ok() == true);
      CHECK(res2.value() == 55);
    }

    SUBCASE("Copy an error result") {
      aww::result<std::string> res1 = aww::result<std::string>::err(aww::result_error("Copy Error"));
      aww::result<std::string> res2 = res1;
      CHECK(res2.is_err() == true);
      CHECK(res2.error().error_message() == "Copy Error");
    }
  }

  // ----------------------------
  // Fancy logic Tests
  // ----------------------------
  TEST_CASE("Different Type Parameters") {
    SUBCASE("Result with int") {
      auto ensure_even = [](int n) -> aww::result<int> {
        if (n % 2 == 0) {
          return aww::result<int>::ok(n);
        } else {
          return aww::result<int>::err(aww::result_error("Number is not even"));
        }
      };
      auto res = ensure_even(10);
      if (res) {
        CHECK(res.value() == 10);
      } else {
        FAIL("Result is_even(10) should be Ok");
      }

      res = ensure_even(7);
      if (not res) {
        CHECK(res.error().error_message() == "Number is not even");
      } else {
        FAIL("Result is_even(7) should be Err");
      }
    }
  }

  // ----------------------------
  // Different Type Parameters Tests
  // ----------------------------
  TEST_CASE("Different Type Parameters") {
    SUBCASE("Result with int") {
      aww::result<int> res = aww::result<int>::ok(10);
      CHECK(res.is_ok() == true);
      CHECK(res.value() == 10);
    }

    SUBCASE("Result with std::string") {
      aww::result<std::string> res = aww::result<std::string>::ok("String Test");
      CHECK(res.is_ok() == true);
      CHECK(res.value() == "String Test");
    }

    SUBCASE("Result with CustomType") {
      CustomType ct{2, "Custom"};
      aww::result<CustomType> res = aww::result<CustomType>::ok(ct);
      CHECK(res.is_ok() == true);
      CHECK(res.value() == ct);
    }
  }

  // ----------------------------
  // Edge Cases Tests
  // ----------------------------
  TEST_CASE("Edge Cases") {
    SUBCASE("Empty error message") {
      aww::result<int> res = aww::result<int>::err(aww::result_error(""));
      CHECK(res.is_err() == true);
      CHECK(res.error().error_message().empty() == true);
    }

    SUBCASE("Large value") {
      long long large_value = 1LL << 60;
      aww::result<long long> res = aww::result<long long>::ok(large_value);
      CHECK(res.is_ok() == true);
      CHECK(res.value() == large_value);
    }

    SUBCASE("Zero value") {
      aww::result<int> res = aww::result<int>::ok(0);
      CHECK(res.is_ok() == true);
      CHECK(res.value() == 0);
    }
  }

  // ----------------------------
  // Factory Methods Tests
  // ----------------------------
  TEST_CASE("Factory Methods") {
    SUBCASE("Ok factory method") {
      aww::result<double> res = aww::result<double>::ok(3.1415);
      CHECK(res.is_ok() == true);
      CHECK(res.value() == doctest::Approx(3.1415));
    }

    SUBCASE("Err factory method") {
      aww::result<double> res = aww::result<double>::err(aww::result_error("Factory Error"));
      CHECK(res.is_err() == true);
      CHECK(res.error().error_message() == "Factory Error");
    }
  }
}
