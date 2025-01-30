#include "aww-pubsub/aww-pubsub.hpp"
#include "doctest/doctest.h"
#include <atomic>
#include <thread>

TEST_SUITE("aww::pubsub_event_bus Tests") {

  // ----------------------------
  // Basic Subscription and Event Publishing
  // ----------------------------
  TEST_CASE("Basic Subscription and Event Publishing") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    int received_value = 0;
    auto subscription = bus.subscribe<test_event>([&](const test_event& e) { received_value = e.value; });

    bus.publish(test_event{42});

    CHECK(received_value == 42);
  }

  // ----------------------------
  // Multiple Subscribers Handling
  // ----------------------------
  TEST_CASE("Multiple Subscribers Handling") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    int count = 0;
    auto subscription1 = bus.subscribe<test_event>([&](const test_event&) { count++; });

    auto subscription2 = bus.subscribe<test_event>([&](const test_event&) { count++; });

    bus.publish(test_event{10});

    CHECK(count == 2);
  }

  // ----------------------------
  // RAII Unsubscription
  // ----------------------------
  TEST_CASE("RAII Unsubscription") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    int count = 0;
    {
      auto subscription = bus.subscribe<test_event>([&](const test_event&) { count++; });
      bus.publish(test_event{10});
      CHECK(count == 1); // Should receive the event
    }

    bus.publish(test_event{20});
    CHECK(count == 1); // Should NOT receive the event after subscription out of scope
  }

  // ----------------------------
  // Manual Unsubscription
  // ----------------------------
  TEST_CASE("Manual Unsubscription") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    int count = 0;
    auto subscription = bus.subscribe<test_event>([&](const test_event&) { count++; });

    subscription.reset(); // Manual unsubscription

    bus.publish(test_event{30});
    CHECK(count == 0); // No events should be received
  }

  // ----------------------------
  // Publishing Without Subscribers
  // ----------------------------
  TEST_CASE("Publishing Without Subscribers") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    CHECK_NOTHROW(bus.publish(test_event{50})); // Should not throw or crash
  }

  // ----------------------------
  // Concurrency Test
  // ----------------------------
  TEST_CASE("Concurrency Test") {
    struct test_event {
      int value;
    };
    aww::pubsub_event_bus bus;

    std::atomic<int> count{0};

    auto subscription = bus.subscribe<test_event>([&](const test_event&) { count++; });

    std::thread t1([&]() {
      for (int i = 0; i < 100; ++i) {
        bus.publish(test_event{i});
      }
    });

    std::thread t2([&]() {
      for (int i = 0; i < 100; ++i) {
        bus.publish(test_event{i});
      }
    });

    t1.join();
    t2.join();

    CHECK(count == 200); // Ensure all events were received
  }
}
