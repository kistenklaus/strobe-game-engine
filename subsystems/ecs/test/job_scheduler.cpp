#include <array>
#include <atomic>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

#include "strobe/ecs/schedule/job_scheduler.hpp"

namespace {

constexpr uint32_t kWorkerCount = 8;
constexpr uint32_t kSlotCount = 32;
constexpr uint32_t kIterations = 500;

} // namespace

TEST(JobScheduler, BasicVoidNoArgsRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<int> x{1};

    scheduler.submit(
        [&x]() noexcept { x.store(2, std::memory_order_release); });

    scheduler.stop();

    EXPECT_EQ(x.load(std::memory_order_acquire), 2) << "iter=" << iter;
  }
}

TEST(JobScheduler, DependencyRunsAfterSourceRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<int> stage{0};
    std::atomic<bool> orderOk{true};

    strobe::ecs::job_id a = scheduler.submit(
        [&]() noexcept { stage.store(1, std::memory_order_release); });

    strobe::ecs::job_id b = scheduler.acquire();
    scheduler.addDependency(b, a);
    scheduler.submit(b, [&]() noexcept {
      if (stage.load(std::memory_order_acquire) != 1) {
        orderOk.store(false, std::memory_order_release);
      }
      stage.store(2, std::memory_order_release);
    });

    scheduler.stop();

    EXPECT_TRUE(orderOk.load(std::memory_order_acquire)) << "iter=" << iter;
    EXPECT_EQ(stage.load(std::memory_order_acquire), 2) << "iter=" << iter;
  }
}

TEST(JobScheduler, MultipleDependenciesRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> completedSources{0};
    std::atomic<bool> consumerSawBoth{false};

    strobe::ecs::job_id a = scheduler.submit([&]() noexcept {
      completedSources.fetch_add(1, std::memory_order_acq_rel);
    });

    strobe::ecs::job_id b = scheduler.submit([&]() noexcept {
      completedSources.fetch_add(1, std::memory_order_acq_rel);
    });

    strobe::ecs::job_id c = scheduler.acquire();
    scheduler.addDependency(c, a);
    scheduler.addDependency(c, b);
    scheduler.submit(c, [&]() noexcept {
      consumerSawBoth.store(completedSources.load(std::memory_order_acquire) ==
                                2,
                            std::memory_order_release);
    });

    scheduler.stop();

    EXPECT_EQ(completedSources.load(std::memory_order_acquire), 2u)
        << "iter=" << iter;
    EXPECT_TRUE(consumerSawBoth.load(std::memory_order_acquire))
        << "iter=" << iter;
  }
}

TEST(JobScheduler, DependencyOnNotYetSubmittedJobRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<int> sourceDone{0};
    std::atomic<bool> orderOk{true};

    strobe::ecs::job_id source = scheduler.acquire();
    strobe::ecs::job_id consumer = scheduler.acquire();

    scheduler.addDependency(consumer, source);

    scheduler.submit(consumer, [&]() noexcept {
      if (sourceDone.load(std::memory_order_acquire) != 1) {
        orderOk.store(false, std::memory_order_release);
      }
    });

    scheduler.submit(source, [&]() noexcept {
      sourceDone.store(1, std::memory_order_release);
    });

    scheduler.stop();

    EXPECT_TRUE(orderOk.load(std::memory_order_acquire)) << "iter=" << iter;
    EXPECT_EQ(sourceDone.load(std::memory_order_acquire), 1) << "iter=" << iter;
  }
}

TEST(JobScheduler, ChildInvocationsRepeated) {
  strobe::ecs::allocator alloc;
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> invocationCount{0};

    scheduler.submit([&](uint32_t invocationId) noexcept -> uint32_t {
      invocationCount.fetch_add(1, std::memory_order_acq_rel);

      if (invocationId == 0) {
        return 7; // total invocations should become 8
      }

      return 0;
    });

    scheduler.stop();

    EXPECT_EQ(invocationCount.load(std::memory_order_acquire), 8u)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, RangeSubmitDependenciesRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> sources{0};
    std::atomic<bool> consumerRan{false};

    strobe::ecs::job_id a = scheduler.submit(
        [&]() noexcept { sources.fetch_add(1, std::memory_order_acq_rel); });

    strobe::ecs::job_id b = scheduler.submit(
        [&]() noexcept { sources.fetch_add(1, std::memory_order_acq_rel); });

    std::array<strobe::ecs::job_id, 2> deps{a, b};

    strobe::ecs::job_id c = scheduler.submit(
        [&]() noexcept {
          consumerRan.store(sources.load(std::memory_order_acquire) == 2,
                            std::memory_order_release);
        },
        deps);

    (void)c;

    scheduler.stop();

    EXPECT_EQ(sources.load(std::memory_order_acquire), 2u) << "iter=" << iter;
    EXPECT_TRUE(consumerRan.load(std::memory_order_acquire)) << "iter=" << iter;
  }
}

TEST(JobScheduler, CapturedSharedPtrReleasedAfterCompletionRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    auto ptr = std::make_shared<int>(42);
    std::weak_ptr<int> weak = ptr;

    scheduler.submit([ptr = std::move(ptr)]() noexcept {});

    scheduler.stop();

    EXPECT_TRUE(weak.expired()) << "iter=" << iter;
  }
}

TEST(JobScheduler, StoredCallableDestructorRunsOnceRepeated) {
  struct Probe {
    std::atomic<uint32_t> *destructions = nullptr;

    explicit Probe(std::atomic<uint32_t> *destructions) noexcept
        : destructions(destructions) {}

    Probe(const Probe &) = delete;
    Probe &operator=(const Probe &) = delete;

    Probe(Probe &&other) noexcept : destructions(other.destructions) {
      other.destructions = nullptr;
    }

    Probe &operator=(Probe &&) = delete;

    void operator()() noexcept {}

    ~Probe() noexcept {
      if (destructions != nullptr) {
        destructions->fetch_add(1, std::memory_order_acq_rel);
      }
    }
  };

  static_assert(strobe::ecs::job_fn<Probe>);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> destructions{0};

    scheduler.submit(Probe{&destructions});

    scheduler.stop();

    EXPECT_EQ(destructions.load(std::memory_order_acquire), 1u)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, ChildInvocationCallableDestroyedOnceRepeated) {
  struct Probe {
    std::atomic<uint32_t> *invocations = nullptr;
    std::atomic<uint32_t> *destructions = nullptr;

    Probe(std::atomic<uint32_t> *invocations,
          std::atomic<uint32_t> *destructions) noexcept
        : invocations(invocations), destructions(destructions) {}

    Probe(const Probe &) = delete;
    Probe &operator=(const Probe &) = delete;

    Probe(Probe &&other) noexcept
        : invocations(other.invocations), destructions(other.destructions) {
      other.invocations = nullptr;
      other.destructions = nullptr;
    }

    Probe &operator=(Probe &&) = delete;

    uint32_t operator()(uint32_t invocationId) noexcept {
      invocations->fetch_add(1, std::memory_order_acq_rel);

      if (invocationId == 0) {
        return 7;
      }

      return 0;
    }

    ~Probe() noexcept {
      if (destructions != nullptr) {
        destructions->fetch_add(1, std::memory_order_acq_rel);
      }
    }
  };

  static_assert(strobe::ecs::job_fn<Probe>);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> invocations{0};
    std::atomic<uint32_t> destructions{0};

    scheduler.submit(Probe{&invocations, &destructions});

    scheduler.stop();

    EXPECT_EQ(invocations.load(std::memory_order_acquire), 8u)
        << "iter=" << iter;
    EXPECT_EQ(destructions.load(std::memory_order_acquire), 1u)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, DependencyCallableCapturesReleasedRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    auto sourcePtr = std::make_shared<int>(1);
    auto consumerPtr = std::make_shared<int>(2);

    std::weak_ptr<int> sourceWeak = sourcePtr;
    std::weak_ptr<int> consumerWeak = consumerPtr;

    strobe::ecs::job_id source =
        scheduler.submit([sourcePtr = std::move(sourcePtr)]() noexcept {});

    strobe::ecs::job_id consumer = scheduler.acquire();
    scheduler.addDependency(consumer, source);
    scheduler.submit(consumer,
                     [consumerPtr = std::move(consumerPtr)]() noexcept {});

    scheduler.stop();

    EXPECT_TRUE(sourceWeak.expired()) << "iter=" << iter;
    EXPECT_TRUE(consumerWeak.expired()) << "iter=" << iter;
  }
}

TEST(JobScheduler, ReusedSlotsDestroyEachCallableRepeated) {
  struct Probe {
    std::atomic<uint32_t> *destructions = nullptr;

    explicit Probe(std::atomic<uint32_t> *destructions) noexcept
        : destructions(destructions) {}

    Probe(const Probe &) = delete;
    Probe &operator=(const Probe &) = delete;

    Probe(Probe &&other) noexcept : destructions(other.destructions) {
      other.destructions = nullptr;
    }

    Probe &operator=(Probe &&) = delete;

    void operator()() noexcept {}

    ~Probe() noexcept {
      if (destructions != nullptr) {
        destructions->fetch_add(1, std::memory_order_acq_rel);
      }
    }
  };

  static_assert(strobe::ecs::job_fn<Probe>);

  constexpr uint32_t kJobsPerIteration = 128;

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> destructions{0};

    for (uint32_t i = 0; i < kJobsPerIteration; ++i) {
      scheduler.submit(Probe{&destructions});
    }

    scheduler.stop();

    EXPECT_EQ(destructions.load(std::memory_order_acquire), kJobsPerIteration)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, ManySourcesOneSinkStaleGenerationsRepeated) {
  constexpr uint32_t kSourceJobs = 1000;
  constexpr uint32_t kLocalSlotCount = 64;

  static_assert(kLocalSlotCount > kWorkerCount);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kLocalSlotCount};

    std::array<strobe::ecs::job_id, kSourceJobs> sources{};

    std::atomic<uint32_t> sourceCount{0};
    std::atomic<bool> sinkRan{false};

    for (uint32_t i = 0; i < kSourceJobs; ++i) {
      sources[i] = scheduler.submit([&]() noexcept {
        sourceCount.fetch_add(1, std::memory_order_acq_rel);
      });
    }

    strobe::ecs::job_id sink = scheduler.acquire();

    for (strobe::ecs::job_id source : sources) {
      scheduler.addDependency(sink, source);
    }

    scheduler.submit(sink, [&]() noexcept {
      sinkRan.store(true, std::memory_order_release);
    });

    scheduler.stop();

    EXPECT_EQ(sourceCount.load(std::memory_order_acquire), kSourceJobs)
        << "iter=" << iter;
    EXPECT_TRUE(sinkRan.load(std::memory_order_acquire)) << "iter=" << iter;
  }
}

TEST(JobScheduler, TrueFanInThousandSourcesRepeated) {
  constexpr uint32_t kSourceJobs = 1000;
  constexpr uint32_t kLocalSlotCount = kSourceJobs + 8;

  static_assert(kLocalSlotCount > kWorkerCount);

  struct SourceJob {
    std::atomic<bool> *releaseSources;
    std::atomic<uint32_t> *sourceCount;

    void operator()() noexcept {
      while (!releaseSources->load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }

      sourceCount->fetch_add(1, std::memory_order_acq_rel);
    }
  };

  struct SinkJob {
    std::atomic<uint32_t> *sourceCount;
    std::atomic<bool> *sinkSawAll;

    void operator()() noexcept {
      sinkSawAll->store(sourceCount->load(std::memory_order_acquire) ==
                            kSourceJobs,
                        std::memory_order_release);
    }
  };

  static_assert(strobe::ecs::job_fn<SourceJob>);
  static_assert(strobe::ecs::job_fn<SinkJob>);

  for (uint32_t iter = 0; iter < 100; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kLocalSlotCount};

    std::array<strobe::ecs::job_id, kSourceJobs> sources{};

    std::atomic<bool> releaseSources{false};
    std::atomic<uint32_t> sourceCount{0};
    std::atomic<bool> sinkSawAll{false};

    for (uint32_t i = 0; i < kSourceJobs; ++i) {
      sources[i] = scheduler.submit(SourceJob{&releaseSources, &sourceCount});
    }

    strobe::ecs::job_id sink = scheduler.acquire();

    for (strobe::ecs::job_id source : sources) {
      scheduler.addDependency(sink, source);
    }

    scheduler.submit(sink, SinkJob{&sourceCount, &sinkSawAll});

    releaseSources.store(true, std::memory_order_release);

    scheduler.stop();

    EXPECT_EQ(sourceCount.load(std::memory_order_acquire), kSourceJobs)
        << "iter=" << iter;
    EXPECT_TRUE(sinkSawAll.load(std::memory_order_acquire)) << "iter=" << iter;
  }
}

TEST(JobScheduler, AddDependencyAfterSourceAlreadyCompletedRepeated) {
  struct SourceJob {
    std::atomic<bool> *done;

    void operator()() noexcept { done->store(true, std::memory_order_release); }
  };

  struct ConsumerJob {
    std::atomic<bool> *sourceDone;
    std::atomic<bool> *orderOk;

    void operator()() noexcept {
      if (!sourceDone->load(std::memory_order_acquire)) {
        orderOk->store(false, std::memory_order_release);
      }
    }
  };

  static_assert(strobe::ecs::job_fn<SourceJob>);
  static_assert(strobe::ecs::job_fn<ConsumerJob>);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<bool> sourceDone{false};
    std::atomic<bool> orderOk{true};

    strobe::ecs::job_id source = scheduler.acquire();
    strobe::ecs::job_id consumer = scheduler.acquire();

    scheduler.submit(source, SourceJob{&sourceDone});

    while (!sourceDone.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    scheduler.addDependency(consumer, source);
    scheduler.submit(consumer, ConsumerJob{&sourceDone, &orderOk});

    scheduler.stop();

    EXPECT_TRUE(orderOk.load(std::memory_order_acquire)) << "iter=" << iter;
  }
}

TEST(JobScheduler, OneSourceManyConsumersRepeated) {
  constexpr uint32_t kConsumers = 256;
  constexpr uint32_t kLocalSlotCount = kConsumers + 16;

  struct State {
    std::atomic<bool> releaseSource{false};
    std::atomic<bool> sourceDone{false};
    std::atomic<uint32_t> consumerCount{0};
    std::atomic<bool> orderOk{true};
  };

  struct SourceJob {
    State *state;

    void operator()() noexcept {
      while (!state->releaseSource.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }

      state->sourceDone.store(true, std::memory_order_release);
    }
  };

  struct ConsumerJob {
    State *state;

    void operator()() noexcept {
      if (!state->sourceDone.load(std::memory_order_acquire)) {
        state->orderOk.store(false, std::memory_order_release);
      }

      state->consumerCount.fetch_add(1, std::memory_order_acq_rel);
    }
  };

  static_assert(sizeof(SourceJob) <= strobe::ecs::MAX_JOB_FN_SIZE);
  static_assert(sizeof(ConsumerJob) <= strobe::ecs::MAX_JOB_FN_SIZE);
  static_assert(strobe::ecs::job_fn<SourceJob>);
  static_assert(strobe::ecs::job_fn<ConsumerJob>);

  for (uint32_t iter = 0; iter < 100; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kLocalSlotCount};

    State state;

    strobe::ecs::job_id source = scheduler.submit(SourceJob{&state});

    for (uint32_t i = 0; i < kConsumers; ++i) {
      strobe::ecs::job_id consumer = scheduler.acquire();
      scheduler.addDependency(consumer, source);
      scheduler.submit(consumer, ConsumerJob{&state});
    }

    state.releaseSource.store(true, std::memory_order_release);

    scheduler.stop();

    EXPECT_TRUE(state.orderOk.load(std::memory_order_acquire))
        << "iter=" << iter;
    EXPECT_EQ(state.consumerCount.load(std::memory_order_acquire), kConsumers)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, LongDependencyChainRepeated) {
  constexpr uint32_t kChainLength = 256;
  constexpr uint32_t kLocalSlotCount = kChainLength + 8;

  for (uint32_t iter = 0; iter < 100; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kLocalSlotCount};

    std::array<std::atomic<uint32_t>, kChainLength> stages{};

    strobe::ecs::job_id previous = scheduler.submit(
        [&]() noexcept { stages[0].store(1, std::memory_order_release); });

    for (uint32_t i = 1; i < kChainLength; ++i) {
      strobe::ecs::job_id current = scheduler.acquire();
      scheduler.addDependency(current, previous);

      scheduler.submit(current, [&, i]() noexcept {
        const uint32_t prev = stages[i - 1].load(std::memory_order_acquire);
        stages[i].store(prev + 1, std::memory_order_release);
      });

      previous = current;
    }

    scheduler.stop();

    EXPECT_EQ(stages[kChainLength - 1].load(std::memory_order_acquire),
              kChainLength)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, DiamondDependencyRepeated) {
  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> stage{0};
    std::atomic<bool> dSawBoth{false};

    strobe::ecs::job_id a = scheduler.submit(
        [&]() noexcept { stage.fetch_add(1, std::memory_order_acq_rel); });

    strobe::ecs::job_id b = scheduler.acquire();
    scheduler.addDependency(b, a);
    scheduler.submit(
        b, [&]() noexcept { stage.fetch_add(1, std::memory_order_acq_rel); });

    strobe::ecs::job_id c = scheduler.acquire();
    scheduler.addDependency(c, a);
    scheduler.submit(
        c, [&]() noexcept { stage.fetch_add(1, std::memory_order_acq_rel); });

    strobe::ecs::job_id d = scheduler.acquire();
    scheduler.addDependency(d, b);
    scheduler.addDependency(d, c);
    scheduler.submit(d, [&]() noexcept {
      dSawBoth.store(stage.load(std::memory_order_acquire) == 3,
                     std::memory_order_release);
    });

    scheduler.stop();

    EXPECT_EQ(stage.load(std::memory_order_acquire), 3u) << "iter=" << iter;
    EXPECT_TRUE(dSawBoth.load(std::memory_order_acquire)) << "iter=" << iter;
  }
}

TEST(JobScheduler, RecursiveChildInvocationTreeRepeated) {
  struct TreeJob {
    std::atomic<uint32_t> *count;

    uint32_t operator()(uint32_t invocationId) noexcept {
      count->fetch_add(1, std::memory_order_acq_rel);

      // Binary tree for invocation IDs:
      // 0 spawns 2
      // 1,2 spawn 2 each
      // 3,4,5,6 spawn 2 each
      // total = 1 + 2 + 4 + 8 = 15
      if (invocationId < 7) {
        return 2;
      }

      return 0;
    }
  };

  static_assert(strobe::ecs::job_fn<TreeJob>);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> count{0};

    scheduler.submit(TreeJob{&count});

    scheduler.stop();

    EXPECT_EQ(count.load(std::memory_order_acquire), 15u) << "iter=" << iter;
  }
}

TEST(JobScheduler, CallableSignatureVariantsRepeated) {
  struct NoArgVoid {
    std::atomic<uint32_t> *count;

    void operator()() noexcept {
      count->fetch_add(1, std::memory_order_acq_rel);
    }
  };

  struct NoArgReturnsChildren {
    std::atomic<uint32_t> *count;

    uint32_t operator()() noexcept {
      const uint32_t old = count->fetch_add(1, std::memory_order_acq_rel);

      return old == 0 ? 3 : 0;
    }
  };

  struct ArgVoid {
    std::atomic<uint32_t> *sum;

    void operator()(uint32_t invocationId) noexcept {
      sum->fetch_add(invocationId, std::memory_order_acq_rel);
    }
  };

  struct ArgReturnsChildren {
    std::atomic<uint32_t> *count;

    uint32_t operator()(uint32_t invocationId) noexcept {
      count->fetch_add(1, std::memory_order_acq_rel);
      return invocationId == 0 ? 2 : 0;
    }
  };

  static_assert(strobe::ecs::job_fn<NoArgVoid>);
  static_assert(strobe::ecs::job_fn<NoArgReturnsChildren>);
  static_assert(strobe::ecs::job_fn<ArgVoid>);
  static_assert(strobe::ecs::job_fn<ArgReturnsChildren>);

  for (uint32_t iter = 0; iter < kIterations; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> noArgVoidCount{0};
    std::atomic<uint32_t> noArgChildrenCount{0};
    std::atomic<uint32_t> argVoidSum{0};
    std::atomic<uint32_t> argChildrenCount{0};

    scheduler.submit(NoArgVoid{&noArgVoidCount});
    scheduler.submit(NoArgReturnsChildren{&noArgChildrenCount});
    scheduler.submit(ArgVoid{&argVoidSum});
    scheduler.submit(ArgReturnsChildren{&argChildrenCount});

    scheduler.stop();

    EXPECT_EQ(noArgVoidCount.load(std::memory_order_acquire), 1u)
        << "iter=" << iter;

    EXPECT_EQ(noArgChildrenCount.load(std::memory_order_acquire), 4u)
        << "iter=" << iter;

    EXPECT_EQ(argVoidSum.load(std::memory_order_acquire), 0u)
        << "iter=" << iter;

    EXPECT_EQ(argChildrenCount.load(std::memory_order_acquire), 3u)
        << "iter=" << iter;
  }
}

TEST(JobScheduler, SlotReusePressureRepeated) {
  constexpr uint32_t kJobs = 4096;

  for (uint32_t iter = 0; iter < 100; ++iter) {
    strobe::ecs::allocator alloc;
    strobe::ecs::JobScheduler scheduler{&alloc, kWorkerCount, kSlotCount};

    std::atomic<uint32_t> count{0};

    for (uint32_t i = 0; i < kJobs; ++i) {
      scheduler.submit(
          [&]() noexcept { count.fetch_add(1, std::memory_order_acq_rel); });
    }

    scheduler.stop();

    EXPECT_EQ(count.load(std::memory_order_acquire), kJobs) << "iter=" << iter;
  }
}
