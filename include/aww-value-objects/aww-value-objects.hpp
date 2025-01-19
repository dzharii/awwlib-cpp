#ifndef AWW_VALUE_OBJECTS_HPP
#define AWW_VALUE_OBJECTS_HPP

#include <compare>
#include <utility>

namespace aww {
/**
 * @brief Abstract base class (DDD Value Object) that wraps a single primitive type T.
 *
 * Characteristics:
 *  - No default construction: must be given a value on creation.
 *  - Immutability: the stored value cannot be modified after construction.
 *  - Equality & ordering: defined purely by the wrapped value (no identity).
 *  - Virtual destructor: suitable for subclassing.
 *
 * @tparam T The primitive type being wrapped (e.g., int, double, etc.).
 */
template <typename T> class ValueObject {
private:
  /**
   * @brief The single wrapped primitive value.
   */
  T _value;

protected:
  /**
   * @brief Construct a ValueObject with a given primitive value.
   *
   * @param value The primitive value to store.
   *
   * Making this constructor protected prevents direct instantiation of
   * ValueObject<T>; users must subclass (or have a friend).
   */
  explicit ValueObject(T value) : _value(std::move(value)) {
  }

public:
  /**
   * @brief Virtual destructor, in-class definition.
   *
   * Making it virtual allows safe polymorphic usage of subclasses.
   */
  virtual ~ValueObject() = default;

  /**
   * @brief Delete the default constructor to avoid creating a ValueObject<T> with no value.
   */
  ValueObject() = delete;

  /**
   * @brief Default copy constructor (freely copyable).
   */
  ValueObject(const ValueObject&) = default;

  /**
   * @brief Default move constructor (noexcept for potential performance benefits).
   */
  ValueObject(ValueObject&&) noexcept = default;

  /**
   * @brief Default copy assignment (overwrites with the other object's value).
   */
  ValueObject& operator=(const ValueObject&) = default;

  /**
   * @brief Default move assignment (noexcept for potential performance benefits).
   */
  ValueObject& operator=(ValueObject&&) noexcept = default;

  /**
   * @brief Spaceship operator provides all comparison operators by default.
   *
   * @param other Another ValueObject to compare against.
   * @return The result of comparing the wrapped values with <=>.
   */
  auto operator<=>(const ValueObject& other) const = default;

  /**
   * @brief Equality operator for comparing two ValueObjects.
   *
   * @param other The other ValueObject to compare.
   * @return true if the wrapped values are equal, false otherwise.
   */
  bool operator==(const ValueObject& other) const {
    return _value == other._value;
  }

  /**
   * @brief Inequality operator for comparing two ValueObjects.
   *
   * @param other The other ValueObject to compare.
   * @return true if the wrapped values are not equal, false otherwise.
   */
  bool operator!=(const ValueObject& other) const {
    return !(*this == other);
  }

  /**
   * @brief Provides read-only access to the wrapped value.
   *
   * @return const reference to the stored primitive.
   */
  [[nodiscard]]
  const T& value() const noexcept {
    return _value;
  }
};

/**
 * Example usage:
 *
 * // A specialized ValueObject that wraps 'int':
 * class NonNegativeInt : public ValueObject<int>
 * {
 * public:
 *     explicit NonNegativeInt(int val)
 *         : ValueObject<int>(val)
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
