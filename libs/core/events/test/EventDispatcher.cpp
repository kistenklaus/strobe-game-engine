#include <gtest/gtest.h>

#include <strobe/core/events/basic_event.hpp>
#include <strobe/core/events/event_dispatcher.hpp>
#include <strobe/core/events/event_listener.hpp>

#include <atomic>
#include <chrono>
#include <semaphore>
#include <thread>
#include <vector>

namespace {

using Event = strobe::BasicEvent<int>;
using Dispatcher = strobe::EventDispatcher<Event>;

class CancelableEvent {
public:
  using payload_type = int;

  explicit CancelableEvent(int payload) : m_payload(payload) {}

  const int &payload() const noexcept { return m_payload; }

  bool canceled() const noexcept { return m_canceled; }

  void cancel() const noexcept { m_canceled = true; }

private:
  int m_payload;
  mutable bool m_canceled = false;
};

static_assert(strobe::events::Event<CancelableEvent>);

using CancelableDispatcher = strobe::EventDispatcher<CancelableEvent>;

} // namespace

TEST(EventDispatcherTest, BasicDispatch) {
  Dispatcher dispatcher;

  int callCount = 0;
  auto listener = strobe::EventListenerRef<Event>::fromNative(
      &callCount, [](void *userData, const Event &) {
        ++*static_cast<int *>(userData);
      });

  auto handle = dispatcher.addListener(listener);

  dispatcher.dispatch(42);

  EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcherTest, RemoveListener) {
  Dispatcher dispatcher;

  int callCount = 0;
  auto handle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &callCount, [](void *userData, const Event &) {
            ++*static_cast<int *>(userData);
          }));

  EXPECT_TRUE(dispatcher.removeListener(handle));
  EXPECT_FALSE(handle);

  dispatcher.dispatch(42);

  EXPECT_EQ(callCount, 0);
  EXPECT_TRUE(dispatcher.empty());
}

TEST(EventDispatcherTest, MultipleListeners) {
  Dispatcher dispatcher;

  int firstCalls = 0;
  int secondCalls = 0;

  auto first = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &firstCalls, [](void *userData, const Event &) {
            ++*static_cast<int *>(userData);
          }));

  auto second = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &secondCalls, [](void *userData, const Event &) {
            ++*static_cast<int *>(userData);
          }));

  dispatcher.dispatch(42);

  EXPECT_EQ(firstCalls, 1);
  EXPECT_EQ(secondCalls, 1);
}

TEST(EventDispatcherTest, CallableAndMemberFunctionListeners) {
  struct Callable {
    int calls = 0;

    void operator()(const Event &) { ++calls; }
  } callable;

  struct Receiver {
    int calls = 0;

    void receive(const Event &) { ++calls; }
  } receiver;

  Dispatcher dispatcher;

  auto callableHandle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromCallable(&callable));

  auto memberHandle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromMemberFunction<
          Receiver, &Receiver::receive>(&receiver));

  dispatcher.dispatch(0);

  EXPECT_EQ(callable.calls, 1);
  EXPECT_EQ(receiver.calls, 1);
}

TEST(EventDispatcherTest, DuplicateListenerRegistrationsAreIndependent) {
  Dispatcher dispatcher;

  int callCount = 0;
  auto listener = strobe::EventListenerRef<Event>::fromNative(
      &callCount, [](void *userData, const Event &) {
        ++*static_cast<int *>(userData);
      });

  auto first = dispatcher.addListener(listener);
  auto second = dispatcher.addListener(listener);

  dispatcher.dispatch(0);
  EXPECT_EQ(callCount, 2);

  first.release();
  dispatcher.dispatch(0);
  EXPECT_EQ(callCount, 3);
}

TEST(EventDispatcherTest, DispatcherCannotRemoveForeignHandle) {
  Dispatcher firstDispatcher;
  Dispatcher secondDispatcher;

  int callCount = 0;
  auto handle = firstDispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &callCount, [](void *userData, const Event &) {
            ++*static_cast<int *>(userData);
          }));

  EXPECT_FALSE(secondDispatcher.removeListener(handle));
  EXPECT_TRUE(handle);

  firstDispatcher.dispatch(0);
  EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcherTest, ListenerCanRemoveItself) {
  struct Context {
    Dispatcher dispatcher;
    strobe::EventListenerHandle self;
    strobe::EventListenerHandle other;
    int selfCalls = 0;
    int otherCalls = 0;
  } context;

  context.self = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            auto &context = *static_cast<Context *>(userData);
            ++context.selfCalls;
            context.self.release();
          }));

  context.other = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            ++static_cast<Context *>(userData)->otherCalls;
          }));

  context.dispatcher.dispatch(0);
  context.dispatcher.dispatch(0);

  EXPECT_EQ(context.selfCalls, 1);
  EXPECT_EQ(context.otherCalls, 2);
}

TEST(EventDispatcherTest, ListenerCanRemoveLaterListener) {
  struct Context {
    Dispatcher dispatcher;
    strobe::EventListenerHandle first;
    strobe::EventListenerHandle second;
    int firstCalls = 0;
    int secondCalls = 0;
  } context;

  context.first = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            auto &context = *static_cast<Context *>(userData);
            ++context.firstCalls;
            context.second.release();
          }));

  context.second = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            ++static_cast<Context *>(userData)->secondCalls;
          }));

  context.dispatcher.dispatch(0);

  EXPECT_EQ(context.firstCalls, 1);
  EXPECT_EQ(context.secondCalls, 0);
}

TEST(EventDispatcherTest, NestedDispatchIsReentrant) {
  struct Context {
    Dispatcher dispatcher;
    strobe::EventListenerHandle first;
    strobe::EventListenerHandle second;
    int firstCalls = 0;
    int secondCalls = 0;
    bool nested = false;
  } context;

  context.first = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            auto &context = *static_cast<Context *>(userData);
            ++context.firstCalls;

            if (!context.nested) {
              context.nested = true;
              context.dispatcher.dispatch(1);
            }
          }));

  context.second = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            ++static_cast<Context *>(userData)->secondCalls;
          }));

  context.dispatcher.dispatch(0);

  EXPECT_EQ(context.firstCalls, 2);
  EXPECT_EQ(context.secondCalls, 2);
}

TEST(EventDispatcherTest,
     ListenerAddedDuringDispatchWaitsForOutermostDispatch) {
  struct Context {
    Dispatcher dispatcher;
    strobe::EventListenerHandle first;
    strobe::EventListenerHandle added;
    int firstCalls = 0;
    int addedCalls = 0;
    bool nested = false;
  } context;

  context.first = context.dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            auto &context = *static_cast<Context *>(userData);
            ++context.firstCalls;

            if (!context.nested) {
              context.nested = true;
              context.added = context.dispatcher.addListener(
                  strobe::EventListenerRef<Event>::fromNative(
                      &context, [](void *userData, const Event &) {
                        ++static_cast<Context *>(userData)->addedCalls;
                      }));

              context.dispatcher.dispatch(1);
            }
          }));

  context.dispatcher.dispatch(0);

  EXPECT_EQ(context.firstCalls, 2);
  EXPECT_EQ(context.addedCalls, 0);

  context.dispatcher.dispatch(0);

  EXPECT_EQ(context.firstCalls, 3);
  EXPECT_EQ(context.addedCalls, 1);
}

TEST(EventDispatcherTest, LowerLayersDispatchFirst) {
  struct Listener {
    std::vector<int> *order;
    int value;
  };

  auto callback = [](void *userData, const Event &) {
    auto &listener = *static_cast<Listener *>(userData);
    listener.order->push_back(listener.value);
  };

  Dispatcher dispatcher;
  std::vector<int> order;

  Listener high{&order, 3};
  Listener low{&order, 1};
  Listener middle{&order, 2};

  auto highHandle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(&high, callback), 192);
  auto lowHandle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(&low, callback), 0);
  auto middleHandle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(&middle, callback), 64);

  dispatcher.dispatch(0);

  EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

TEST(EventDispatcherTest, CancellationTakesEffectBetweenLayers) {
  struct Context {
    int cancelingLayerCalls = 0;
    int peerLayerCalls = 0;
    int higherLayerCalls = 0;
  } context;

  CancelableDispatcher dispatcher;

  auto canceling = dispatcher.addListener(
      strobe::EventListenerRef<CancelableEvent>::fromNative(
          &context, [](void *userData, const CancelableEvent &event) {
            ++static_cast<Context *>(userData)->cancelingLayerCalls;
            event.cancel();
          }),
      16);

  auto peer = dispatcher.addListener(
      strobe::EventListenerRef<CancelableEvent>::fromNative(
          &context, [](void *userData, const CancelableEvent &) {
            ++static_cast<Context *>(userData)->peerLayerCalls;
          }),
      16);

  auto higher = dispatcher.addListener(
      strobe::EventListenerRef<CancelableEvent>::fromNative(
          &context, [](void *userData, const CancelableEvent &) {
            ++static_cast<Context *>(userData)->higherLayerCalls;
          }),
      17);

  dispatcher.dispatch(0);

  EXPECT_EQ(context.cancelingLayerCalls, 1);
  EXPECT_EQ(context.peerLayerCalls, 1);
  EXPECT_EQ(context.higherLayerCalls, 0);
}

TEST(EventDispatcherTest, HandleMayOutliveDispatcher) {
  strobe::EventListenerHandle handle;
  int callCount = 0;

  {
    Dispatcher dispatcher;
    handle = dispatcher.addListener(
        strobe::EventListenerRef<Event>::fromNative(
            &callCount, [](void *userData, const Event &) {
              ++*static_cast<int *>(userData);
            }));
  }

  EXPECT_TRUE(handle);
  EXPECT_FALSE(handle.release());
  EXPECT_FALSE(handle);
}

TEST(EventDispatcherTest, HandleDestructionWaitsForRunningCallback) {
  struct Context {
    std::binary_semaphore entered{0};
    std::binary_semaphore continueCallback{0};
    std::atomic<int> calls{0};
  } context;

  Dispatcher dispatcher;
  auto handle = dispatcher.addListener(
      strobe::EventListenerRef<Event>::fromNative(
          &context, [](void *userData, const Event &) {
            auto &context = *static_cast<Context *>(userData);
            context.calls.fetch_add(1, std::memory_order_relaxed);
            context.entered.release();
            context.continueCallback.acquire();
          }));

  std::thread dispatchThread([&] { dispatcher.dispatch(0); });
  context.entered.acquire();

  std::atomic<bool> releaseReturned{false};
  std::binary_semaphore releaseStarted{0};

  std::thread releaseThread([&] {
    releaseStarted.release();
    handle.release();
    releaseReturned.store(true, std::memory_order_release);
  });

  releaseStarted.acquire();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_FALSE(releaseReturned.load(std::memory_order_acquire));

  context.continueCallback.release();

  dispatchThread.join();
  releaseThread.join();

  EXPECT_TRUE(releaseReturned.load(std::memory_order_acquire));

  dispatcher.dispatch(0);
  EXPECT_EQ(context.calls.load(std::memory_order_relaxed), 1);
}

TEST(EventDispatcherTest, EmptyDispatch) {
  Dispatcher dispatcher;

  dispatcher.dispatch(42);

  EXPECT_TRUE(dispatcher.empty());
}
