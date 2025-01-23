#ifndef AWW_RESULT_HPP
#define AWW_RESULT_HPP

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

namespace aww {

/**
 * @struct result_error
 * @brief Represents an error with a specific message. (aww tag #jyjhwmmy084)
 *
 * The `result_error` struct encapsulates error information, including a descriptive error message.
 * It provides accessor methods to retrieve these details.
 */
struct result_error {
public:
  /**
   * @brief Constructs a `result_error` with the specified message.
   *
   * Initializes the error message by moving the provided `std::string` to avoid unnecessary copying.
   *
   * @param message The descriptive message associated with the error.
   */
  result_error(std::string message) : m_message(std::move(message)) {
  }

  /**
   * @brief Retrieves the error message.
   *
   * @return A constant reference to the `std::string` containing the error message.
   */
  const std::string& error_message() const noexcept {
    return m_message;
  }

private:
  std::string m_message; /**< The descriptive error message. */
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
  static result ok(T value) {
    return result(std::move(value));
  }

  /**
   * @brief Creates an error Result containing the provided error.
   * @param error The ResultError to store in the Result.
   * @return A Result object representing an error.
   */
  static result err(result_error error) {
    return result(std::move(error));
  }

  /**
   * @brief Creates an error Result containing the provided error message.
   * @param error The error message to store in the Result.
   * @return A Result object representing an error.
   */
  static result err(std::string error) {
    return result(result_error(std::move(error)));
  }

  /**
   * @brief Creates an error Result containing the provided exception.
   * @param error The exception to store in the Result.
   * @return A Result object representing an error.
   */
  static result err(const std::exception& error) {
    return result(result_error(error.what()));
  }

  // Check if the result is success
  bool is_ok() const noexcept {
    return std::holds_alternative<T>(m_result);
  }

  // Check if the result is an error
  bool is_err() const noexcept {
    return !is_ok();
  }

  // Implicit conversion to bool (true if success)
  explicit operator bool() const noexcept {
    return is_ok();
  }

  // Get the value by const reference
  const T& value() const& {
    if (is_ok()) {
      return std::get<T>(m_result);
    }
    throw std::runtime_error("Attempted to get value from an error result");
  }

  // Get the value by rvalue reference (allows moving)
  T&& value() && {
    if (is_ok()) {
      return std::move(std::get<T>(m_result));
    }
    throw std::runtime_error("Attempted to get value from an error result");
  }

  // Get the error by const reference
  const result_error& error() const& {
    if (is_err()) {
      return std::get<result_error>(m_result);
    }
    throw std::runtime_error("Attempted to get error from a success result");
  }

  // Get the error by rvalue reference (allows moving)
  result_error&& error() && {
    if (is_err()) {
      return std::move(std::get<result_error>(m_result));
    }
    throw std::runtime_error("Attempted to get error from a success result");
  }

private:
  // Private constructors to enforce the use of factory methods
  explicit result(T value) : m_result(std::move(value)) {
  }
  explicit result(result_error error) : m_result(std::move(error)) {
  }

  std::variant<T, result_error> m_result;
};

} // namespace aww

#endif // AWW_RESULT_HPP
