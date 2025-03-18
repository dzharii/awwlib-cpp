#include "aww-spsc-queue/aww-spsc-queue.hpp"
#include "doctest/doctest.h"

#include <chrono>
#include <thread>
#include <vector>

using namespace aww;

TEST_CASE("Empty Queue Test") {
  single_producer_single_consumer_queue<int, 8> q;
  // Initially the queue is empty.
  auto res = q.pop();
  CHECK_FALSE(res.has_value());
}

TEST_CASE("Single Push and Pop Test") {
  single_producer_single_consumer_queue<int, 8> q;
  // Push one element and pop it.
  CHECK(q.push(42));
  auto res = q.pop();
  CHECK(res.has_value());
  CHECK_EQ(*res, 42);
  // After the pop, the queue should be empty.
  CHECK_FALSE(q.pop().has_value());
}

TEST_CASE("Full Queue Behavior Test") {
  // Using a queue of size 8 (effective capacity = 7)
  single_producer_single_consumer_queue<int, 8> q;
  // Fill the queue to capacity.
  for (int i = 0; i < 7; i++) {
    CHECK(q.push(i));
  }
  // At this point, an additional push would block waiting for space.
  // We run the extra push in a separate thread.
  bool extraPushSucceeded = false;
  std::thread producer([&]() {
    // This push will block until a consumer pops an element.
    extraPushSucceeded = q.push(99);
  });
  // Allow some time for the producer thread to enter wait state.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Pop one element to free up space.
  auto popped = q.pop();
  CHECK(popped.has_value());
  // Wait for the producer to complete.
  producer.join();
  // The blocked push should now succeed.
  CHECK(extraPushSucceeded);

  // Now, pop all remaining elements and verify the order.
  std::vector<int> values;
  while (auto opt = q.pop()) {
    values.push_back(*opt);
  }
  // Expected order:
  // Initially pushed: 0, 1, 2, 3, 4, 5, 6.
  // One element (0) was popped to free space, then 99 was pushed.
  // The remaining elements should be: 1, 2, 3, 4, 5, 6, 99.
  std::vector<int> expected = {1, 2, 3, 4, 5, 6, 99};
  CHECK_EQ(values, expected);
}

TEST_CASE("Wrap-around Behavior Test") {
  // Use a smaller queue for easier wrap-around testing (size 5 => capacity = 4).
  single_producer_single_consumer_queue<int, 5> q;
  // Initial state: front_ = 4, back_ = 0.
  // Push three elements.
  for (int i = 0; i < 3; i++) {
    CHECK(q.push(i));
  }
  // Pop one element; should retrieve the first pushed element.
  auto popped = q.pop();
  CHECK(popped.has_value());
  CHECK_EQ(*popped, 0);
  // Now push two more elements to force wrap-around.
  CHECK(q.push(3));
  CHECK(q.push(4)); // After this push, the queue becomes full (4 elements stored).

  // Pop all remaining elements.
  std::vector<int> values;
  while (auto opt = q.pop()) {
    values.push_back(*opt);
  }
  // Expected order:
  // Initially pushed: 0, 1, 2; 0 was popped.
  // Then pushed: 3, 4.
  // Remaining order: 1, 2, 3, 4.
  std::vector<int> expected = {1, 2, 3, 4};
  CHECK_EQ(values, expected);
}

TEST_CASE("Concurrent Producer Consumer Test") {
  // Use a larger queue for concurrency testing.
  single_producer_single_consumer_queue<int, 1024> q;
  const int num_items = 10000;
  std::vector<int> consumed;
  consumed.reserve(num_items);

  // Producer thread: pushes num_items sequential integers.
  std::thread producer([&]() {
    for (int i = 0; i < num_items; i++) {
      while (!q.push(i)) {
        // Yield to allow the consumer to catch up.
        std::this_thread::yield();
      }
    }
  });

  // Consumer thread: pops num_items items and stores them.
  std::thread consumer([&]() {
    int count = 0;
    while (count < num_items) {
      auto res = q.pop();
      if (res.has_value()) {
        consumed.push_back(*res);
        count++;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  // Verify that all numbers were consumed in order.
  CHECK_EQ(consumed.size(), static_cast<size_t>(num_items));
  for (int i = 0; i < num_items; i++) {
    CHECK_EQ(consumed[i], i);
  }
}
