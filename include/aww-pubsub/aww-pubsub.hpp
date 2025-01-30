#ifndef AWW_PUBSUB_HPP
#define AWW_PUBSUB_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace aww {

/**
 * @brief Strongly typed subscriber ID for safety.
 */
struct pubsub_subscriber_id {
  unsigned int value;

  // Comparison operators (for erase_if, etc.)
  bool operator==(const pubsub_subscriber_id& other) const {
    return value == other.value;
  }

  bool operator!=(const pubsub_subscriber_id& other) const {
    return value != other.value;
  }
};

/**
 * @brief An RAII-style subscription handle that automatically unsubscribes
 *        when destroyed (unless you call `release()`).
 */
class pubsub_subscription {
public:
  pubsub_subscription() noexcept = default;

  explicit pubsub_subscription(std::function<void()> unsubscribe_func) : m_unsubscribe(std::move(unsubscribe_func)) {
  }

  // No copy
  pubsub_subscription(const pubsub_subscription&) = delete;
  pubsub_subscription& operator=(const pubsub_subscription&) = delete;

  // Move support
  pubsub_subscription(pubsub_subscription&& other) noexcept : m_unsubscribe(std::move(other.m_unsubscribe)) {
    other.m_unsubscribe = {};
  }

  pubsub_subscription& operator=(pubsub_subscription&& other) noexcept {
    if (this != &other) {
      reset();
      m_unsubscribe = std::move(other.m_unsubscribe);
      other.m_unsubscribe = {};
    }
    return *this;
  }

  /**
   * @brief Unsubscribe immediately.
   */
  void reset() {
    if (m_unsubscribe) {
      m_unsubscribe();
      m_unsubscribe = {};
    }
  }

  /**
   * @brief Prevent this subscription from auto-unsubscribing on destruction,
   *        transferring responsibility to the caller.
   */
  void release() {
    m_unsubscribe = {};
  }

  /**
   * @brief Destructor unsubscribes if still active.
   */
  ~pubsub_subscription() {
    reset();
  }

private:
  std::function<void()> m_unsubscribe;
};

// Forward declaration
class pubsub_event_bus;

/**
 * @brief Internal concept of a "Topic" that handles one particular event type.
 */
struct pubsub_topic_interface {
  virtual ~pubsub_topic_interface() = default;
};

/**
 * @brief Templated topic implementation for a specific event type.
 */
template <typename T> class pubsub_topic final : public pubsub_topic_interface {
public:
  using callback_t = std::function<void(const T&)>;

  pubsub_subscription subscribe(callback_t callback) {
    std::lock_guard<std::mutex> lock(m_mutex);

    pubsub_subscriber_id id{++m_last_id};
    m_subscribers.emplace_back(id, std::move(callback));

    return pubsub_subscription([this, id]() { this->unsubscribe(id); });
  }

  void publish(const T& event) {
    std::vector<callback_t> callbacks;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      callbacks.reserve(m_subscribers.size());
      for (const auto& [sub_id, fn] : m_subscribers) {
        callbacks.push_back(fn);
      }
    }
    for (const auto& fn : callbacks) {
      fn(event);
    }
  }

private:
  void unsubscribe(pubsub_subscriber_id id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it =
        std::remove_if(m_subscribers.begin(), m_subscribers.end(),
                       [id](const std::pair<pubsub_subscriber_id, callback_t>& item) { return item.first == id; });
    if (it != m_subscribers.end()) {
      m_subscribers.erase(it, m_subscribers.end());
    }
  }

private:
  mutable std::mutex m_mutex;
  unsigned int m_last_id{0};
  std::vector<std::pair<pubsub_subscriber_id, callback_t>> m_subscribers;
};

/**
 * @brief A thread-safe event bus that can publish multiple event types.
 */
class pubsub_event_bus {
public:
  /**
   * @brief Subscribe to events of type T.
   * @param callback The callback function. Must be callable as `void(const T&)`.
   * @return An RAII `pubsub_subscription` that unsubscribes when destroyed.
   */
  template <typename T> pubsub_subscription subscribe(std::function<void(const T&)> callback) {
    std::lock_guard<std::mutex> lock(m_map_mutex);

    std::type_index type_idx = std::type_index(typeid(T));
    auto it = m_topics.find(type_idx);
    if (it == m_topics.end()) {
      auto inserted = m_topics.emplace(type_idx, std::make_unique<pubsub_topic<T>>());
      it = inserted.first;
    }

    pubsub_topic<T>* typed_topic = static_cast<pubsub_topic<T>*>(it->second.get());
    return typed_topic->subscribe(std::move(callback));
  }

  /**
   * @brief Publish an event of type T. All registered subscribers for T are called.
   */
  template <typename T> void publish(const T& event) {
    pubsub_topic_interface* topic_ptr = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_map_mutex);
      std::type_index type_idx = std::type_index(typeid(T));
      auto it = m_topics.find(type_idx);
      if (it == m_topics.end()) {
        return;
      }
      topic_ptr = it->second.get();
    }
    pubsub_topic<T>* typed_topic = static_cast<pubsub_topic<T>*>(topic_ptr);
    typed_topic->publish(event);
  }

private:
  std::mutex m_map_mutex;
  std::unordered_map<std::type_index, std::unique_ptr<pubsub_topic_interface>> m_topics;
};

} // namespace aww
#endif // AWW_PUBSUB_HPP
