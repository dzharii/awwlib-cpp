#include "aww-value-objects/aww-value-objects.hpp"
#include "doctest/doctest.h"
#include <algorithm> // For std::sort
#include <stdexcept>
#include <string>
#include <vector>

namespace aww {

// Example specialized ValueObjects for testing

/**
 * @brief NonNegativeInt Value Object: ensures an integer is non-negative.
 */
class NonNegativeInt : public ValueObject<int> {
public:
  explicit NonNegativeInt(int val) : ValueObject<int>(val) {
    if (val < 0) {
      throw std::invalid_argument("NonNegativeInt must not be negative");
    }
  }
};

/**
 * @brief EmailAddress Value Object: ensures a string contains '@'.
 */
class EmailAddress : public ValueObject<std::string> {
public:
  explicit EmailAddress(const std::string& val) : ValueObject<std::string>(val) {
    if (val.find('@') == std::string::npos) {
      throw std::invalid_argument("EmailAddress must contain '@'");
    }
  }
};

/**
 * @brief USAddress Value Object: ensures all address fields are non-empty.
 */
struct USAddressFields {
  std::string Street;
  std::string City;
  std::string State;
  std::string ZipCode;

  bool operator==(const USAddressFields& other) const = default;
  auto operator<=>(const USAddressFields& other) const = default;
};

class USAddress : public ValueObject<USAddressFields> {
public:
  explicit USAddress(const USAddressFields& fields) : ValueObject<USAddressFields>(fields) {
    if (fields.Street.empty()) {
      throw std::invalid_argument("Street field must not be empty");
    }
    if (fields.City.empty()) {
      throw std::invalid_argument("City field must not be empty");
    }
    if (fields.State.empty()) {
      throw std::invalid_argument("State field must not be empty");
    }
    if (fields.ZipCode.empty()) {
      throw std::invalid_argument("ZipCode field must not be empty");
    }
  }
};

} // namespace aww

TEST_CASE("ValueObject<T> usage examples and behavior specifications") {
  using namespace aww;

  SUBCASE("Construction and immutability") {
    NonNegativeInt validInt(10);
    CHECK(validInt.value() == 10);

    // Invalid construction
    CHECK_THROWS_AS(NonNegativeInt(-5), std::invalid_argument);
    CHECK_THROWS_WITH(NonNegativeInt(-5), "NonNegativeInt must not be negative");
  }

  SUBCASE("Equality and ordering") {
    NonNegativeInt a(5);
    NonNegativeInt b(5);
    NonNegativeInt c(10);

    // Equality

    CHECK(a == b);
    CHECK(a != c);

    // Ordering (uses spaceship operator)
    CHECK(a < c);
    CHECK(c > b);
    CHECK(a <= b);
    CHECK(c >= b);
  }

  SUBCASE("Subclass with custom validation (EmailAddress)") {
    EmailAddress email("user@example.com");
    CHECK(email.value() == "user@example.com");

    // Invalid construction
    CHECK_THROWS_AS(EmailAddress("invalid-email"), std::invalid_argument);
    CHECK_THROWS_WITH(EmailAddress("invalid-email"), "EmailAddress must contain '@'");

    // Equality for EmailAddress
    EmailAddress email1("user@example.com");
    EmailAddress email2("user@example.com");
    EmailAddress email3("admin@example.com");
    CHECK(email1 == email2);
    CHECK(email1 != email3);
  }

  SUBCASE("Copy and move semantics") {
    NonNegativeInt original(42);
    NonNegativeInt copy = original;
    NonNegativeInt moved = std::move(original);

    // Copy preserves the same value
    CHECK(copy.value() == 42);

    // Move preserves the same value
    CHECK(moved.value() == 42);
  }

  SUBCASE("Combining value objects in operations") {
    NonNegativeInt a(3);
    NonNegativeInt b(7);

    // Demonstrate creating a new object as a result of an operation
    NonNegativeInt sum(a.value() + b.value());
    CHECK(sum.value() == 10);
  }

  SUBCASE("Using value objects in standard containers") {
    // Ensure that STL containers work with ValueObjects
    std::vector<NonNegativeInt> numbers = {NonNegativeInt(3), NonNegativeInt(1), NonNegativeInt(2)};
    CHECK(numbers.size() == 3);

    // Accessing elements
    CHECK(numbers[0].value() == 3);
    CHECK(numbers[1].value() == 1);
    CHECK(numbers[2].value() == 2);

    // Sorting (spaceship operator ensures this works)
    std::sort(numbers.begin(), numbers.end());
    CHECK(numbers[0].value() == 1);
    CHECK(numbers[1].value() == 2);
    CHECK(numbers[2].value() == 3);
  }

  SUBCASE("USAddress with validation") {
    USAddress validAddress({"123 Main St", "Springfield", "IL", "62704"});

    CHECK(validAddress.value().Street == "123 Main St");
    CHECK(validAddress.value().City == "Springfield");
    CHECK(validAddress.value().State == "IL");
    CHECK(validAddress.value().ZipCode == "62704");

    // Invalid construction: specific field validation
    CHECK_THROWS_WITH(USAddress({"", "Springfield", "IL", "62704"}), "Street field must not be empty");
    CHECK_THROWS_WITH(USAddress({"123 Main St", "", "IL", "62704"}), "City field must not be empty");
    CHECK_THROWS_WITH(USAddress({"123 Main St", "Springfield", "", "62704"}), "State field must not be empty");
    CHECK_THROWS_WITH(USAddress({"123 Main St", "Springfield", "IL", ""}), "ZipCode field must not be empty");

    // Equality
    USAddress address1({"123 Main St", "Springfield", "IL", "62704"});
    USAddress address2({"123 Main St", "Springfield", "IL", "62704"});
    USAddress address3({"456 Elm St", "Springfield", "IL", "62704"});

    CHECK(address1 == address2);
    CHECK(address1 != address3);
  }
}
