#include <gtest/gtest.h>
#include <strobe/core/events/event_dispatcher.hpp>
#include <strobe/core/events/basic_event.hpp>
#include <strobe/core/events/event_listener.hpp>

namespace strobe::event_dispatcher::testing {

using namespace strobe;

} // namespace strobe::event_dispatcher::testing

using namespace strobe::event_dispatcher::testing;

TEST(EventDispatcherTest, BasicDispatch) {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

    int callCount = 0;
    auto listener = EventListenerRef<BasicEvent<int>>::fromNative(&callCount, +[](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });

    dispatcher.addListener(listener);

    dispatcher.dispatch(42);

    EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcherTest, RemoveListener) {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

    int callCount = 0;
    auto listener = EventListenerRef<BasicEvent<int>>::fromNative(&callCount, +[](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
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
    auto listener1 = EventListenerRef<BasicEvent<int>>::fromNative(&callCount1, +[](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });
    auto listener2 = EventListenerRef<BasicEvent<int>>::fromNative(&callCount2, +[](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });

    dispatcher.addListener(listener1);
    dispatcher.addListener(listener2);

    dispatcher.dispatch(42);

    EXPECT_EQ(callCount1, 1);
    EXPECT_EQ(callCount2, 1);
}

TEST(EventDispatcherTest, RemoveListenerDuringDispatch) {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

    int callCount = 0;
    EventListenerHandle handle;
    auto listener = EventListenerRef<BasicEvent<int>>::fromNative(&callCount, [](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });

    handle = dispatcher.addListener(listener);

    dispatcher.removeListener(handle);

    dispatcher.dispatch(42);

    EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcherTest, AddListenerDuringDispatch) {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

    int callCount = 0;
    auto listener1 = EventListenerRef<BasicEvent<int>>::fromNative(&callCount, [](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });

    dispatcher.addListener(listener1);

    dispatcher.dispatch(42);

    EXPECT_EQ(callCount, 1);

    auto listener2 = EventListenerRef<BasicEvent<int>>::fromNative(&callCount, +[](void* userData, const BasicEvent<int>& e) {
        int* counter = static_cast<int*>(userData);
        (*counter)++;
    });

    dispatcher.addListener(listener2);

    dispatcher.dispatch(42);

    EXPECT_EQ(callCount, 3);
}

TEST(EventDispatcherTest, EmptyDispatch) {
    EventDispatcher<BasicEvent<int>, strobe::Mallocator> dispatcher;

    dispatcher.dispatch(42); // No listeners, should not crash
}
