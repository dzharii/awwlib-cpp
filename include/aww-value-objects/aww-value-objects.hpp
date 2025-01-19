#ifndef AWW_VALUE_OBJECTS_HPP
#define AWW_VALUE_OBJECTS_HPP

#include <compare>
#include <utility>

namespace aww {
/**
 * @brief Abstract base class (DDD Value Object) that wraps a single primitive type T. (aww tag #bw9ercgm8cf)
 *
 * Characteristics:
 *  - No default construction: must be given a value on creation.
 *  - Immutability: the stored value cannot be modified after construction.
 *  - Equality & ordering: defined purely by the wrapped value (no identity).
 *  - Virtual destructor: suitable for subclassing.
 *
 * @tparam T The primitive type being wrapped (e.g., int, double, etc.).
 */
template <typename T> class value_object {
private:
  /**
   * @brief The single wrapped primitive value.
   */
  T m_value;

protected:
  /**
   * @brief Construct a value_object with a given primitive value.
   *
   * @param value The primitive value to store.
   *
   * Making this constructor protected prevents direct instantiation of
   * value_object<T>; users must subclass (or have a friend).
   */
  explicit value_object(T value) : m_value(std::move(value)) {
  }

public:
  /**
   * @brief Virtual destructor, in-class definition.
   *
   * Making it virtual allows safe polymorphic usage of subclasses.
   */
  virtual ~value_object() = default;

  /**
   * @brief Delete the default constructor to avoid creating a value_object<T> with no value.
   */
  value_object() = delete;

  /**
   * @brief Default copy constructor (freely copyable).
   */
  value_object(const value_object&) = default;

  /**
   * @brief Default move constructor (noexcept for potential performance benefits).
   */
  value_object(value_object&&) noexcept = default;

  /**
   * @brief Default copy assignment (overwrites with the other object's value).
   */
  value_object& operator=(const value_object&) = default;

  /**
   * @brief Default move assignment (noexcept for potential performance benefits).
   */
  value_object& operator=(value_object&&) noexcept = default;

  /**
   * @brief Spaceship operator provides all comparison operators by default.
   *
   * @param other Another value_object to compare against.
   * @return The result of comparing the wrapped values with <=>.
   */
  auto operator<=>(const value_object& other) const = default;

  /**
   * @brief Equality operator for comparing two ValueObjects.
   *
   * @param other The other value_object to compare.
   * @return true if the wrapped values are equal, false otherwise.
   */
  bool operator==(const value_object& other) const {
    return m_value == other.m_value;
  }

  /**
   * @brief Inequality operator for comparing two ValueObjects.
   *
   * @param other The other value_object to compare.
   * @return true if the wrapped values are not equal, false otherwise.
   */
  bool operator!=(const value_object& other) const {
    return !(*this == other);
  }

  /**
   * @brief Provides read-only access to the wrapped value.
   *
   * @return const reference to the stored primitive.
   */
  [[nodiscard]]
  const T& value() const noexcept {
    return m_value;
  }
};

/**
 * Example usage:
 *
 * // A specialized value_object that wraps 'int':
 * class NonNegativeInt : public value_object<int>
 * {
 * public:
 *     explicit NonNegativeInt(int val)
 *         : value_object<int>(val)
 *     {
 *         if (val < 0)
 *         {
 *             throw std::invalid_argument("NonNegativeInt must not be negative");
 *         }
 *     }
 * };
 *
 * // Now NonNegativeInt can be used safely:
 * NonNegativeInt x(5), y(10);
 * if (x < y) { // uses operator<=> internally
 *     // ...
 * }
 * // x == y is also valid, courtesy of the operator<=>(...).
 */

} // namespace aww
#endif // AWW_VALUE_OBJECTS_HPP
