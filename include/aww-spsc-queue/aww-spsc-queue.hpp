#ifndef AWW_SPSC_QUEUE_HPP
#define AWW_SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <optional>

namespace aww {

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4324) // disable warning about structure padding due to alignas
#endif

/**
 * @brief A lock-free, single-producer single-consumer (SPSC) ring buffer queue. (aww tag #51wb1rn83g0)
 * The effective capacity is N-1, since one slot is reserved to distinguish between empty and full states.
 * Heavily inspired by
 * https://github.com/PacktPublishing/Hands-On-Design-Patterns-with-CPP-Second-Edition/blob/main/Chapter18/21_lock_free_queue.C
 * by Fedor G. Pikus
 */
template <typename T, std::size_t N>
  requires std::movable<T>
class single_producer_single_consumer_queue {
  static_assert(N > 1, "Queue size must be greater than 1 to differentiate between empty and full states");

  // Use std::array for safer container management.
  std::array<T, N> m_buffer;

  // Wrapper for atomic indices with explicit cache-line alignment.
  struct alignas(64) atomic_index {
    std::atomic<std::size_t> value;
  };

  // The atomic indices for managing the ring buffer.
  atomic_index m_back{{0}};      // Next write index (producer writes)
  atomic_index m_front{{N - 1}}; // Last read index (consumer writes)

public:
  single_producer_single_consumer_queue() = default;
  single_producer_single_consumer_queue(const single_producer_single_consumer_queue&) = delete;
  single_producer_single_consumer_queue& operator=(const single_producer_single_consumer_queue&) = delete;

  /// Push an item into the queue.
  /// Blocks until space is available if the queue is full.
  template <typename U> bool push(U&& item) {
    std::size_t back = m_back.value.load(std::memory_order_relaxed);
    std::size_t front = m_front.value.load(std::memory_order_acquire);
    // If the queue is full, wait until space is available.
    while (back == front) {
      m_front.value.wait(front, std::memory_order_acquire);
      front = m_front.value.load(std::memory_order_acquire);
    }
    // Write the item into the current back slot.
    m_buffer[back] = std::forward<U>(item);
    m_back.value.store((back + 1) % N, std::memory_order_release);
    m_back.value.notify_one();
    return true;
  }

  /// Pop an item from the queue.
  /// Returns std::nullopt immediately if the queue is empty.
  std::optional<T> pop() {
    const std::size_t front = m_front.value.load(std::memory_order_relaxed);
    const std::size_t next = (front + 1) % N;
    const std::size_t back = m_back.value.load(std::memory_order_acquire);
    // If the queue is empty, return immediately.
    if (next == back) [[unlikely]] {
      return std::nullopt;
    }
    std::optional<T> result{std::move(m_buffer[next])};
    m_front.value.store(next, std::memory_order_release);
    m_front.value.notify_one();
    return result;
  }

  [[nodiscard]]
  std::size_t front() {
    return m_front.value.load(std::memory_order_relaxed);
  }

  [[nodiscard]]
  std::size_t back() {
    return m_back.value.load(std::memory_order_relaxed);
  }
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} // namespace aww

#endif // AWW_SPSC_QUEUE_HPP
