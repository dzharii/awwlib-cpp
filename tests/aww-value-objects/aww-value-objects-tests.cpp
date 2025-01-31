#include "aww-value-objects/aww-value-objects.hpp"
#include "doctest/doctest.h"
#include <algorithm> // For std::sort
#include <stdexcept>
#include <string>
#include <vector>

namespace aww {

// Example specialized ValueObjects for testing

/**
 * @brief non_negative_int Value Object: ensures an integer is non-negative.
 */
class non_negative_int : public value_object<int> {
public:
  explicit non_negative_int(int val) : value_object<int>(val) {
    if (val < 0) {
      throw std::invalid_argument("non_negative_int must not be negative");
    }
  }
};

/**
 * @brief email_address Value Object: ensures a string contains '@'.
 */
class email_address : public value_object<std::string> {
public:
  explicit email_address(const std::string& val) : value_object<std::string>(val) {
    if (val.find('@') == std::string::npos) {
      throw std::invalid_argument("email_address must contain '@'");
    }
  }
};

/**
 * @brief us_address Value Object: ensures all address fields are non-empty.
 */
struct us_address_fields {
  std::string street;
  std::string city;
  std::string state;
  std::string zip_code;

  bool operator==(const us_address_fields& other) const = default;
  auto operator<=>(const us_address_fields& other) const = default;
};

class us_address : public value_object<us_address_fields> {
public:
  explicit us_address(const us_address_fields& fields) : value_object<us_address_fields>(fields) {
    if (fields.street.empty()) {
      throw std::invalid_argument("street field must not be empty");
    }
    if (fields.city.empty()) {
      throw std::invalid_argument("city field must not be empty");
    }
    if (fields.state.empty()) {
      throw std::invalid_argument("state field must not be empty");
    }
    if (fields.zip_code.empty()) {
      throw std::invalid_argument("zip_code field must not be empty");
    }
  }
};

} // namespace aww

TEST_CASE("value_object<T> usage examples and behavior specifications") {
  using namespace aww;

  SUBCASE("Construction and immutability") {
    non_negative_int validInt(10);
    CHECK(validInt.value() == 10);

    // Invalid construction
    CHECK_THROWS_AS(non_negative_int(-5), std::invalid_argument);
    CHECK_THROWS_WITH(non_negative_int(-5), "non_negative_int must not be negative");
  }

  SUBCASE("Equality and ordering") {
    non_negative_int a(5);
    non_negative_int b(5);
    non_negative_int c(10);

    // Equality

    CHECK(a == b);
    CHECK(a != c);

    // Ordering (uses spaceship operator)
    CHECK(a < c);
    CHECK(c > b);
    CHECK(a <= b);
    CHECK(c >= b);
  }

  SUBCASE("Subclass with custom validation (email_address)") {
    email_address email("user@example.com");
    CHECK(email.value() == "user@example.com");

    // Invalid construction
    CHECK_THROWS_AS(email_address("invalid-email"), std::invalid_argument);
    CHECK_THROWS_WITH(email_address("invalid-email"), "email_address must contain '@'");

    // Equality for email_address
    email_address email1("user@example.com");
    email_address email2("user@example.com");
    email_address email3("admin@example.com");
    CHECK(email1 == email2);
    CHECK(email1 != email3);
  }

  SUBCASE("Copy and move semantics") {
    non_negative_int original(42);
    non_negative_int copy = original;
    non_negative_int moved = std::move(original);

    // Copy preserves the same value
    CHECK(copy.value() == 42);

    // Move preserves the same value
    CHECK(moved.value() == 42);
  }

  SUBCASE("Combining value objects in operations") {
    non_negative_int a(3);
    non_negative_int b(7);

    // Demonstrate creating a new object as a result of an operation
    non_negative_int sum(a.value() + b.value());
    CHECK(sum.value() == 10);
  }

  SUBCASE("Using value objects in standard containers") {
    // Ensure that STL containers work with ValueObjects
    std::vector<non_negative_int> numbers = {non_negative_int(3), non_negative_int(1), non_negative_int(2)};
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

  SUBCASE("us_address with validation") {
    us_address validAddress({"123 Main St", "Springfield", "IL", "62704"});

    CHECK(validAddress.value().street == "123 Main St");
    CHECK(validAddress.value().city == "Springfield");
    CHECK(validAddress.value().state == "IL");
    CHECK(validAddress.value().zip_code == "62704");

    // Invalid construction: specific field validation
    CHECK_THROWS_WITH(us_address({"", "Springfield", "IL", "62704"}), "street field must not be empty");
    CHECK_THROWS_WITH(us_address({"123 Main St", "", "IL", "62704"}), "city field must not be empty");
    CHECK_THROWS_WITH(us_address({"123 Main St", "Springfield", "", "62704"}), "state field must not be empty");
    CHECK_THROWS_WITH(us_address({"123 Main St", "Springfield", "IL", ""}), "zip_code field must not be empty");

    // Equality
    us_address address1({"123 Main St", "Springfield", "IL", "62704"});
    us_address address2({"123 Main St", "Springfield", "IL", "62704"});
    us_address address3({"456 Elm St", "Springfield", "IL", "62704"});

    CHECK(address1 == address2);
    CHECK(address1 != address3);
  }
}

TEST_CASE("value_object<T> goes out of scope") {
  using namespace aww;

  SUBCASE("Primitive type as T") {
    int primitive_value;
    {
      non_negative_int obj(42);
      primitive_value = obj.value();
      CHECK(primitive_value == 42);
    } // obj goes out of scope here
    CHECK(primitive_value == 42);
  }

  SUBCASE("Vector as T") {
    std::vector<int> vector_value;
    class vector_wrapper : public value_object<std::vector<int>> {
    public:
      explicit vector_wrapper(const std::vector<int>& val) : value_object<std::vector<int>>(val) {
      }
    };

    {
      vector_wrapper obj({1, 2, 3});
      vector_value = obj.value();
      CHECK(vector_value == std::vector<int>({1, 2, 3}));
    } // obj goes out of scope here
    CHECK(vector_value == std::vector<int>({1, 2, 3}));
  }

  SUBCASE("String as T") {
    std::string string_value;
    {
      email_address obj("user@example.com");
      string_value = obj.value();
      CHECK(string_value == "user@example.com");
    } // obj goes out of scope here
    CHECK(string_value == "user@example.com");
  }

  SUBCASE("Struct as T") {
    us_address_fields struct_value;
    {
      us_address obj({"123 Main St", "Springfield", "IL", "62704"});
      struct_value = obj.value();
      CHECK(struct_value.street == "123 Main St");
      CHECK(struct_value.city == "Springfield");
      CHECK(struct_value.state == "IL");
      CHECK(struct_value.zip_code == "62704");
    } // obj goes out of scope here
    CHECK(struct_value.street == "123 Main St");
    CHECK(struct_value.city == "Springfield");
    CHECK(struct_value.state == "IL");
    CHECK(struct_value.zip_code == "62704");
  }
}
