#include <gtest/gtest.h>

#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/lifetime/lifetime_registry.hpp"

#include <algorithm>
#include <array>
#include <vector>

namespace {

struct RecordingHook final : strobe::ecs::lifetime_hook {
  std::vector<int> *log = nullptr;
  int id = 0;

  RecordingHook(std::vector<int> &log_, int id_) noexcept
      : log(&log_), id(id_) {}

  void enter(strobe::ecs::Scheduler *) noexcept override { log->push_back(id); }

  void exit(strobe::ecs::Scheduler *) noexcept override { log->push_back(-id); }
};

inline constexpr auto no_deps = [](auto &) noexcept {};

} // namespace

namespace {

template <typename T> bool contains_once(const std::vector<T> &xs, T value) {
  return std::count(xs.begin(), xs.end(), value) == 1;
}

template <typename T> std::size_t index_of(const std::vector<T> &xs, T value) {
  auto it = std::find(xs.begin(), xs.end(), value);
  EXPECT_NE(it, xs.end());
  return static_cast<std::size_t>(std::distance(xs.begin(), it));
}

} // namespace

TEST(LifetimeRegistry, DependencyFreeConstructDestructCanRepeat) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};

  auto a_id = lreg.alloc(&a, no_deps);

  lreg.construct(&scheduler, a_id);
  lreg.destruct(&scheduler, a_id);

  lreg.construct(&scheduler, a_id);
  lreg.destruct(&scheduler, a_id);

  lreg.free(a_id);

  EXPECT_EQ(log, (std::vector<int>{1, -1, 1, -1}));
}

TEST(LifetimeRegistry, DependentWaitsUntilDependencyConstructs) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);

  EXPECT_TRUE(log.empty());

  lreg.construct(&scheduler, dependency_id);

  EXPECT_EQ(log, (std::vector<int>{1, 2}));

  lreg.destruct(&scheduler, dependent_id);
  lreg.destruct(&scheduler, dependency_id);

  lreg.free(dependent_id);
  lreg.free(dependency_id);
}

TEST(LifetimeRegistry, DependencyEntersBeforeDependent) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook foo{log, 1};
  RecordingHook system{log, 2};

  auto foo_id = lreg.alloc(&foo, no_deps);

  auto system_id = lreg.alloc(
      &system, [foo_id](auto &deps) noexcept { deps.require(foo_id); });

  lreg.construct(&scheduler, system_id);
  lreg.construct(&scheduler, foo_id);

  EXPECT_EQ(log, (std::vector<int>{1, 2}));

  lreg.destruct(&scheduler, system_id);
  lreg.destruct(&scheduler, foo_id);

  lreg.free(system_id);
  lreg.free(foo_id);
}

TEST(LifetimeRegistry, DependentExitsBeforeDependency) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook foo{log, 1};
  RecordingHook system{log, 2};

  auto foo_id = lreg.alloc(&foo, no_deps);

  auto system_id = lreg.alloc(
      &system, [foo_id](auto &deps) noexcept { deps.require(foo_id); });

  lreg.construct(&scheduler, foo_id);
  lreg.construct(&scheduler, system_id);

  log.clear();

  lreg.destruct(&scheduler, foo_id);

  EXPECT_EQ(log, (std::vector<int>{-2, -1}));

  lreg.destruct(&scheduler, system_id);

  lreg.free(system_id);
  lreg.free(foo_id);
}

TEST(LifetimeRegistry, TransitiveEnterAndExitOrder) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};

  auto a_id = lreg.alloc(&a, no_deps);

  auto b_id =
      lreg.alloc(&b, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto c_id =
      lreg.alloc(&c, [b_id](auto &deps) noexcept { deps.require(b_id); });

  lreg.construct(&scheduler, c_id);
  lreg.construct(&scheduler, b_id);
  lreg.construct(&scheduler, a_id);

  EXPECT_EQ(log, (std::vector<int>{1, 2, 3}));

  log.clear();

  lreg.destruct(&scheduler, a_id);

  EXPECT_EQ(log, (std::vector<int>{-3, -2, -1}));

  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, DestructBlockedLifetimePreventsLaterEnter) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);

  EXPECT_TRUE(log.empty());

  lreg.destruct(&scheduler, dependent_id);

  lreg.construct(&scheduler, dependency_id);

  EXPECT_EQ(log, (std::vector<int>{1}));

  lreg.destruct(&scheduler, dependency_id);

  lreg.free(dependent_id);
  lreg.free(dependency_id);
}

TEST(LifetimeRegistry, FreeBlockedDependentThenDependencyConstructs) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);
  lreg.destruct(&scheduler, dependent_id);
  lreg.free(dependent_id);

  lreg.construct(&scheduler, dependency_id);
  lreg.destruct(&scheduler, dependency_id);
  lreg.free(dependency_id);

  EXPECT_EQ(log, (std::vector<int>{1, -1}));
}

TEST(LifetimeRegistry, DependencyCanToggleAndDependentFollows) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);

  lreg.construct(&scheduler, dependency_id);
  lreg.destruct(&scheduler, dependency_id);

  lreg.construct(&scheduler, dependency_id);
  lreg.destruct(&scheduler, dependency_id);

  lreg.destruct(&scheduler, dependent_id);

  lreg.free(dependent_id);
  lreg.free(dependency_id);

  EXPECT_EQ(log, (std::vector<int>{
                     1,
                     2,
                     -2,
                     -1,
                     1,
                     2,
                     -2,
                     -1,
                 }));
}

TEST(LifetimeRegistry, DependentRequiresTwoDependencies) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};

  auto a_id = lreg.alloc(&a, no_deps);
  auto b_id = lreg.alloc(&b, no_deps);

  auto c_id = lreg.alloc(&c, [a_id, b_id](auto &deps) noexcept {
    deps.require(a_id);
    deps.require(b_id);
  });

  lreg.construct(&scheduler, c_id);

  lreg.construct(&scheduler, a_id);
  EXPECT_EQ(log, (std::vector<int>{1}));

  lreg.construct(&scheduler, b_id);
  EXPECT_EQ(log, (std::vector<int>{1, 2, 3}));

  log.clear();

  lreg.destruct(&scheduler, a_id);
  EXPECT_EQ(log, (std::vector<int>{-3, -1}));

  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, OneDependencyMultipleDependents) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook a{log, 2};
  RecordingHook b{log, 3};

  auto dep_id = lreg.alloc(&dependency, no_deps);

  auto a_id =
      lreg.alloc(&a, [dep_id](auto &deps) noexcept { deps.require(dep_id); });

  auto b_id =
      lreg.alloc(&b, [dep_id](auto &deps) noexcept { deps.require(dep_id); });

  lreg.construct(&scheduler, a_id);
  lreg.construct(&scheduler, b_id);
  lreg.construct(&scheduler, dep_id);

  EXPECT_EQ(log.size(), 3u);
  EXPECT_EQ(log[0], 1);
  EXPECT_TRUE((log[1] == 2 && log[2] == 3) || (log[1] == 3 && log[2] == 2));

  log.clear();

  lreg.destruct(&scheduler, dep_id);

  EXPECT_EQ(log.size(), 3u);
  EXPECT_EQ(log[2], -1);
  EXPECT_TRUE((log[0] == -2 && log[1] == -3) || (log[0] == -3 && log[1] == -2));

  lreg.destruct(&scheduler, a_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(a_id);
  lreg.free(b_id);
  lreg.free(dep_id);
}

TEST(LifetimeRegistry, FreeLiveDependentThenDependencyExits) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dep_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id = lreg.alloc(
      &dependent, [dep_id](auto &deps) noexcept { deps.require(dep_id); });

  lreg.construct(&scheduler, dep_id);
  lreg.construct(&scheduler, dependent_id);

  log.clear();

  lreg.destruct(&scheduler, dependent_id);
  lreg.free(dependent_id);

  lreg.destruct(&scheduler, dep_id);
  lreg.free(dep_id);

  EXPECT_EQ(log, (std::vector<int>{-2, -1}));
}

TEST(LifetimeRegistry, DiamondDependency) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};
  RecordingHook d{log, 4};

  auto a_id = lreg.alloc(&a, no_deps);

  auto b_id =
      lreg.alloc(&b, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto c_id =
      lreg.alloc(&c, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto d_id = lreg.alloc(&d, [b_id, c_id](auto &deps) noexcept {
    deps.require(b_id);
    deps.require(c_id);
  });

  lreg.construct(&scheduler, d_id);
  lreg.construct(&scheduler, c_id);
  lreg.construct(&scheduler, b_id);
  lreg.construct(&scheduler, a_id);

  ASSERT_EQ(log.size(), 4u);
  EXPECT_EQ(log[0], 1);
  EXPECT_EQ(log[3], 4);

  log.clear();

  lreg.destruct(&scheduler, a_id);

  ASSERT_EQ(log.size(), 4u);
  EXPECT_EQ(log[0], -4);
  EXPECT_EQ(log[3], -1);

  lreg.destruct(&scheduler, d_id);
  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(d_id);
  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, DependentAllocatedAfterLiveDependencyEntersImmediately) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  lreg.construct(&scheduler, dependency_id);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);

  EXPECT_EQ(log, (std::vector<int>{1, 2}));

  lreg.destruct(&scheduler, dependent_id);
  lreg.destruct(&scheduler, dependency_id);

  lreg.free(dependent_id);
  lreg.free(dependency_id);
}

TEST(LifetimeRegistry, TwoDependenciesOneTogglesDependentFollows) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};

  auto a_id = lreg.alloc(&a, no_deps);
  auto b_id = lreg.alloc(&b, no_deps);

  auto c_id = lreg.alloc(&c, [a_id, b_id](auto &deps) noexcept {
    deps.require(a_id);
    deps.require(b_id);
  });

  lreg.construct(&scheduler, c_id);

  lreg.construct(&scheduler, a_id);
  EXPECT_EQ(log, (std::vector<int>{1}));

  lreg.construct(&scheduler, b_id);
  EXPECT_EQ(log, (std::vector<int>{1, 2, 3}));

  log.clear();

  lreg.destruct(&scheduler, a_id);
  EXPECT_EQ(log, (std::vector<int>{-3, -1}));

  lreg.construct(&scheduler, a_id);
  EXPECT_EQ(log, (std::vector<int>{-3, -1, 1, 3}));

  log.clear();

  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, a_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, ThreeDependenciesConstructedInArbitraryOrder) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};
  RecordingHook d{log, 4};

  auto a_id = lreg.alloc(&a, no_deps);
  auto b_id = lreg.alloc(&b, no_deps);
  auto c_id = lreg.alloc(&c, no_deps);

  auto d_id = lreg.alloc(&d, [a_id, b_id, c_id](auto &deps) noexcept {
    deps.require(a_id);
    deps.require(b_id);
    deps.require(c_id);
  });

  lreg.construct(&scheduler, d_id);

  lreg.construct(&scheduler, c_id);
  lreg.construct(&scheduler, a_id);

  EXPECT_EQ(log, (std::vector<int>{3, 1}));

  lreg.construct(&scheduler, b_id);

  EXPECT_EQ(log, (std::vector<int>{3, 1, 2, 4}));

  lreg.destruct(&scheduler, d_id);
  lreg.destruct(&scheduler, b_id);
  lreg.destruct(&scheduler, a_id);
  lreg.destruct(&scheduler, c_id);

  lreg.free(d_id);
  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, OneDependencyManyDependents) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};

  constexpr std::size_t dependent_count = 16;

  std::array<RecordingHook, dependent_count> dependents = {
      RecordingHook{log, 100}, RecordingHook{log, 101}, RecordingHook{log, 102},
      RecordingHook{log, 103}, RecordingHook{log, 104}, RecordingHook{log, 105},
      RecordingHook{log, 106}, RecordingHook{log, 107}, RecordingHook{log, 108},
      RecordingHook{log, 109}, RecordingHook{log, 110}, RecordingHook{log, 111},
      RecordingHook{log, 112}, RecordingHook{log, 113}, RecordingHook{log, 114},
      RecordingHook{log, 115},
  };

  auto dep_id = lreg.alloc(&dependency, no_deps);

  std::array<strobe::ecs::lifetime_id, dependent_count> ids{};

  for (std::size_t i = 0; i < dependent_count; ++i) {
    ids[i] = lreg.alloc(&dependents[i], [dep_id](auto &deps) noexcept {
      deps.require(dep_id);
    });

    lreg.construct(&scheduler, ids[i]);
  }

  lreg.construct(&scheduler, dep_id);

  ASSERT_EQ(log.size(), dependent_count + 1);
  EXPECT_EQ(log.front(), 1);

  for (std::size_t i = 0; i < dependent_count; ++i) {
    EXPECT_TRUE(contains_once(log, 100 + static_cast<int>(i)));
  }

  log.clear();

  lreg.destruct(&scheduler, dep_id);

  ASSERT_EQ(log.size(), dependent_count + 1);
  EXPECT_EQ(log.back(), -1);

  for (std::size_t i = 0; i < dependent_count; ++i) {
    EXPECT_TRUE(contains_once(log, -(100 + static_cast<int>(i))));
  }

  for (auto id : ids) {
    lreg.destruct(&scheduler, id);
  }

  for (auto id : ids) {
    lreg.free(id);
  }

  lreg.free(dep_id);
}

TEST(LifetimeRegistry, ManyDependenciesOneDependent) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  constexpr std::size_t dependency_count = 12;

  std::array<RecordingHook, dependency_count> dependencies = {
      RecordingHook{log, 10}, RecordingHook{log, 11}, RecordingHook{log, 12},
      RecordingHook{log, 13}, RecordingHook{log, 14}, RecordingHook{log, 15},
      RecordingHook{log, 16}, RecordingHook{log, 17}, RecordingHook{log, 18},
      RecordingHook{log, 19}, RecordingHook{log, 20}, RecordingHook{log, 21},
  };

  RecordingHook dependent{log, 100};

  std::array<strobe::ecs::lifetime_id, dependency_count> dep_ids{};

  for (std::size_t i = 0; i < dependency_count; ++i) {
    dep_ids[i] = lreg.alloc(&dependencies[i], no_deps);
  }

  auto dependent_id = lreg.alloc(&dependent, [&](auto &deps) noexcept {
    for (auto id : dep_ids) {
      deps.require(id);
    }
  });

  lreg.construct(&scheduler, dependent_id);

  for (std::size_t i = 0; i + 1 < dependency_count; ++i) {
    lreg.construct(&scheduler, dep_ids[i]);
  }

  EXPECT_EQ(std::count(log.begin(), log.end(), 100), 0);

  lreg.construct(&scheduler, dep_ids.back());

  EXPECT_EQ(std::count(log.begin(), log.end(), 100), 1);
  EXPECT_EQ(log.back(), 100);

  lreg.destruct(&scheduler, dependent_id);

  for (auto id : dep_ids) {
    lreg.destruct(&scheduler, id);
  }

  lreg.free(dependent_id);

  for (auto id : dep_ids) {
    lreg.free(id);
  }
}

TEST(LifetimeRegistry, FreeDependencyWhileDependentIsBlocked) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependent_id);

  EXPECT_TRUE(log.empty());

  lreg.free(dependency_id);

  lreg.destruct(&scheduler, dependent_id);
  lreg.free(dependent_id);

  EXPECT_TRUE(log.empty());
}

TEST(LifetimeRegistry, FreeDependencyAfterItDisabledDependent) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook dependent{log, 2};

  auto dependency_id = lreg.alloc(&dependency, no_deps);

  auto dependent_id =
      lreg.alloc(&dependent, [dependency_id](auto &deps) noexcept {
        deps.require(dependency_id);
      });

  lreg.construct(&scheduler, dependency_id);
  lreg.construct(&scheduler, dependent_id);

  log.clear();

  lreg.destruct(&scheduler, dependency_id);

  EXPECT_EQ(log, (std::vector<int>{-2, -1}));

  lreg.free(dependency_id);

  lreg.destruct(&scheduler, dependent_id);
  lreg.free(dependent_id);
}

TEST(LifetimeRegistry, FreeManyDependentsThenToggleDependency) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};
  RecordingHook a{log, 2};
  RecordingHook b{log, 3};
  RecordingHook c{log, 4};

  auto dep_id = lreg.alloc(&dependency, no_deps);

  auto a_id =
      lreg.alloc(&a, [dep_id](auto &deps) noexcept { deps.require(dep_id); });
  auto b_id =
      lreg.alloc(&b, [dep_id](auto &deps) noexcept { deps.require(dep_id); });
  auto c_id =
      lreg.alloc(&c, [dep_id](auto &deps) noexcept { deps.require(dep_id); });

  lreg.construct(&scheduler, a_id);
  lreg.construct(&scheduler, b_id);
  lreg.construct(&scheduler, c_id);

  lreg.destruct(&scheduler, a_id);
  lreg.destruct(&scheduler, b_id);
  lreg.destruct(&scheduler, c_id);

  lreg.free(a_id);
  lreg.free(b_id);
  lreg.free(c_id);

  lreg.construct(&scheduler, dep_id);
  lreg.destruct(&scheduler, dep_id);
  lreg.free(dep_id);

  EXPECT_EQ(log, (std::vector<int>{1, -1}));
}

TEST(LifetimeRegistry, DiamondCanToggleRootRepeatedly) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};
  RecordingHook d{log, 4};

  auto a_id = lreg.alloc(&a, no_deps);

  auto b_id =
      lreg.alloc(&b, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto c_id =
      lreg.alloc(&c, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto d_id = lreg.alloc(&d, [b_id, c_id](auto &deps) noexcept {
    deps.require(b_id);
    deps.require(c_id);
  });

  lreg.construct(&scheduler, d_id);
  lreg.construct(&scheduler, c_id);
  lreg.construct(&scheduler, b_id);

  for (int i = 0; i < 5; ++i) {
    log.clear();

    lreg.construct(&scheduler, a_id);

    ASSERT_EQ(log.size(), 4u);
    EXPECT_EQ(log.front(), 1);
    EXPECT_EQ(log.back(), 4);

    EXPECT_TRUE(contains_once(log, 2));
    EXPECT_TRUE(contains_once(log, 3));

    log.clear();

    lreg.destruct(&scheduler, a_id);

    ASSERT_EQ(log.size(), 4u);
    EXPECT_EQ(log.front(), -4);
    EXPECT_EQ(log.back(), -1);

    EXPECT_TRUE(contains_once(log, -2));
    EXPECT_TRUE(contains_once(log, -3));
  }

  lreg.destruct(&scheduler, d_id);
  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(d_id);
  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, LargerLayeredDagEnterExitPartialOrder) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook a{log, 1};
  RecordingHook b{log, 2};
  RecordingHook c{log, 3};
  RecordingHook d{log, 4};
  RecordingHook e{log, 5};
  RecordingHook f{log, 6};
  RecordingHook g{log, 7};

  auto a_id = lreg.alloc(&a, no_deps);
  auto b_id = lreg.alloc(&b, no_deps);

  auto c_id =
      lreg.alloc(&c, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto d_id =
      lreg.alloc(&d, [a_id](auto &deps) noexcept { deps.require(a_id); });

  auto e_id =
      lreg.alloc(&e, [b_id](auto &deps) noexcept { deps.require(b_id); });

  auto f_id = lreg.alloc(&f, [c_id, d_id, e_id](auto &deps) noexcept {
    deps.require(c_id);
    deps.require(d_id);
    deps.require(e_id);
  });

  auto g_id =
      lreg.alloc(&g, [f_id](auto &deps) noexcept { deps.require(f_id); });

  lreg.construct(&scheduler, g_id);
  lreg.construct(&scheduler, f_id);
  lreg.construct(&scheduler, e_id);
  lreg.construct(&scheduler, d_id);
  lreg.construct(&scheduler, c_id);

  lreg.construct(&scheduler, b_id);
  EXPECT_FALSE(contains_once(log, 6));
  EXPECT_FALSE(contains_once(log, 7));

  lreg.construct(&scheduler, a_id);

  ASSERT_EQ(log.size(), 7u);

  EXPECT_LT(index_of(log, 1), index_of(log, 3));
  EXPECT_LT(index_of(log, 1), index_of(log, 4));
  EXPECT_LT(index_of(log, 2), index_of(log, 5));
  EXPECT_LT(index_of(log, 3), index_of(log, 6));
  EXPECT_LT(index_of(log, 4), index_of(log, 6));
  EXPECT_LT(index_of(log, 5), index_of(log, 6));
  EXPECT_LT(index_of(log, 6), index_of(log, 7));

  log.clear();

  lreg.destruct(&scheduler, a_id);

  ASSERT_EQ(log.size(), 5u);

  EXPECT_LT(index_of(log, -7), index_of(log, -6));
  EXPECT_LT(index_of(log, -6), index_of(log, -3));
  EXPECT_LT(index_of(log, -6), index_of(log, -4));
  EXPECT_EQ(log.back(), -1);

  lreg.destruct(&scheduler, g_id);
  lreg.destruct(&scheduler, f_id);
  lreg.destruct(&scheduler, e_id);
  lreg.destruct(&scheduler, d_id);
  lreg.destruct(&scheduler, c_id);
  lreg.destruct(&scheduler, b_id);

  lreg.free(g_id);
  lreg.free(f_id);
  lreg.free(e_id);
  lreg.free(d_id);
  lreg.free(c_id);
  lreg.free(b_id);
  lreg.free(a_id);
}

TEST(LifetimeRegistry, ChurnManyBlockedDependents) {
  strobe::ecs::allocator alloc;
  strobe::ecs::Scheduler scheduler{&alloc, 4};
  strobe::ecs::LifetimeRegistry lreg{&alloc};

  std::vector<int> log;

  RecordingHook dependency{log, 1};

  auto dep_id = lreg.alloc(&dependency, no_deps);

  constexpr int count = 1 << 10;

  std::vector<RecordingHook> hooks;
  hooks.reserve(count);

  std::vector<strobe::ecs::lifetime_id> ids;
  ids.reserve(count);

  for (int i = 0; i < count; ++i) {
    hooks.emplace_back(log, 1000 + i);

    ids.push_back(lreg.alloc(&hooks.back(), [dep_id](auto &deps) noexcept {
      deps.require(dep_id);
    }));

    lreg.construct(&scheduler, ids.back());
  }

  EXPECT_TRUE(log.empty());

  for (auto id : ids) {
    lreg.destruct(&scheduler, id);
    lreg.free(id);
  }

  lreg.construct(&scheduler, dep_id);
  lreg.destruct(&scheduler, dep_id);
  lreg.free(dep_id);

  EXPECT_EQ(log, (std::vector<int>{1, -1}));
}
