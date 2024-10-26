#pragma once
#ifndef AWW_RESULT_HPP
#define AWW_RESULT_HPP

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

namespace aww {

/**
 * @struct result_error
 * @brief Represents an error with a specific code and message. (aww tag #jyjhwmmy084)
 *
 * The `result_error` struct encapsulates error information, including an integer
 * error code and a descriptive error message. It provides accessor methods to
 * retrieve these details.
 */
struct result_error {
public:
  /**
   * @brief Constructs a `result_error` with the specified code and message.
   *
   * Initializes the error code and error message by moving the provided
   * `std::string` to avoid unnecessary copying.
   *
   * @param code The integer representing the error code.
   * @param message The descriptive message associated with the error.
   */
  result_error(int code, std::string message) : code_(code), message_(std::move(message)) {}

  /**
   * @brief Retrieves the error code.
   *
   * @return An integer representing the error code.
   */
  int error_code() const noexcept { return code_; }

  /**
   * @brief Retrieves the error message.
   *
   * @return A constant reference to the `std::string` containing the error message.
   */
  const std::string& error_message() const noexcept { return message_; }

private:
  int code_;            /**< The integer error code. */
  std::string message_; /**< The descriptive error message. */
};

/**
 * @brief A template class representing a result that can either be a success (holding a value of type T)
 *        or an error (holding a ResultError). (aww tag #8qnv7916n9k)
 */
template <typename T> class result {
public:
  /**
   * @brief Creates a successful Result containing the provided value.
   * @param value The value to store in the Result.
   * @return A Result object representing success.
   */
  static result Ok(T value) { return result(std::move(value)); }

  /**
   * @brief Creates an error Result containing the provided error.
   * @param error The ResultError to store in the Result.
   * @return A Result object representing an error.
   */
  static result Err(result_error error) { return result(std::move(error)); }

  // Check if the result is success
  bool is_ok() const noexcept { return std::holds_alternative<T>(result_); }

  // Check if the result is an error
  bool is_err() const noexcept { return !is_ok(); }

  // Implicit conversion to bool (true if success)
  explicit operator bool() const noexcept { return is_ok(); }

  // Get the value by const reference
  const T& value() const& {
    if (is_ok()) {
      return std::get<T>(result_);
    }
    throw std::runtime_error("Attempted to get value from an error result");
  }

  // Get the value by rvalue reference (allows moving)
  T&& value() && {
    if (is_ok()) {
      return std::move(std::get<T>(result_));
    }
    throw std::runtime_error("Attempted to get value from an error result");
  }

  // Get the error by const reference
  const result_error& error() const& {
    if (is_err()) {
      return std::get<result_error>(result_);
    }
    throw std::runtime_error("Attempted to get error from a success result");
  }

  // Get the error by rvalue reference (allows moving)
  result_error&& error() && {
    if (is_err()) {
      return std::move(std::get<result_error>(result_));
    }
    throw std::runtime_error("Attempted to get error from a success result");
  }

private:
  // Private constructors to enforce the use of factory methods
  explicit result(T value) : result_(std::move(value)) {}
  explicit result(result_error error) : result_(std::move(error)) {}

  std::variant<T, result_error> result_;
};

} // namespace aww

#endif // AWW_RESULT_HPP
