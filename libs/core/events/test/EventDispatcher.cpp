#include <gtest/gtest.h>

#include <strobe/core/events/basic_event.hpp>
#include <strobe/core/events/event_dispatcher.hpp>
#include <strobe/core/events/event_listener.hpp>

namespace strobe::event_dispatcher::testing {

using namespace strobe;

} // namespace strobe::event_dispatcher::testing

using namespace strobe::event_dispatcher::testing;

TEST(EventDispatcherTest, BasicDispatch) {
  EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

  int callCount = 0;
  auto listener = EventListenerRef<BasicEvent<int>>::fromNative(
      &callCount, [](void *userData, const BasicEvent<int> &) {
        int *counter = static_cast<int *>(userData);
        (*counter)++;
      });

  auto handler = dispatcher.addListener(listener);

  dispatcher.dispatch(42);

  EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcherTest, RemoveListener) {
  EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

  int callCount = 0;
  auto listener = EventListenerRef<BasicEvent<int>>::fromNative(
      &callCount, +[](void *userData, const BasicEvent<int> &) {
        int *counter = static_cast<int *>(userData);
        (*counter)++;
      });

  auto handle = dispatcher.addListener(listener);

  dispatcher.removeListener(handle);

  dispatcher.dispatch(42);

  EXPECT_EQ(callCount, 0);
}

TEST(EventDispatcherTest, MultipleListeners) {
  EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

  int callCount1 = 0, callCount2 = 0;
  auto listener1 = EventListenerRef<BasicEvent<int>>::fromNative(
      &callCount1, +[](void *userData, const BasicEvent<int> &) {
        int *counter = static_cast<int *>(userData);
        (*counter)++;
      });
  auto listener2 = EventListenerRef<BasicEvent<int>>::fromNative(
      &callCount2, +[](void *userData, const BasicEvent<int> &) {
        int *counter = static_cast<int *>(userData);
        (*counter)++;
      });

  auto h1 = dispatcher.addListener(listener1);
  auto h2 = dispatcher.addListener(listener2);

  dispatcher.dispatch(42);

  EXPECT_EQ(callCount1, 1);
  EXPECT_EQ(callCount2, 1);
}

TEST(EventDispatcherTest, RemoveListenerDuringDispatch) {
  struct Container {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;
    int callCount = 0;
    EventListenerHandle handle;
  };

  Container c;
  c.handle =
      c.dispatcher.addListener(EventListenerRef<BasicEvent<int>>::fromNative(
          static_cast<void *>(&c), [](void *userData, const BasicEvent<int> &) {
            auto c = static_cast<Container *>(userData);
            c->callCount++;
            c->handle.release();
          }));

  c.dispatcher.dispatch(0);
  c.dispatcher.dispatch(0);

  EXPECT_EQ(c.callCount, 1);
}

TEST(EventDispatcherTest, AddListenerDuringDispatch) {
  struct Container {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;
    int callCountA = 0;
    int callCountB = 0;
    EventListenerHandle handleA;
    EventListenerHandle handleB;
  };

  Container c;
  c.handleA =
      c.dispatcher.addListener(EventListenerRef<BasicEvent<int>>::fromNative(
          static_cast<void *>(&c),
          [](void *userData, const BasicEvent<int> &) {
            auto c = static_cast<Container *>(userData);
            c->callCountA++;
            if (c->callCountA == 1) {
              c->handleB = c->dispatcher.addListener(
                  EventListenerRef<BasicEvent<int>>::fromNative(
                      c, [](void *userData, const BasicEvent<int> &) {
                        auto c = static_cast<Container *>(userData);
                        c->callCountB++;
                      }));
            }
          }));

  c.dispatcher.dispatch(0);
  c.dispatcher.dispatch(0);

  EXPECT_EQ(c.callCountA, 2);
  EXPECT_EQ(c.callCountB, 1);
}

TEST(EventDispatcherTest, EmptyDispatch) {
  EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

  dispatcher.dispatch(42); // No listeners, should not crash
}
