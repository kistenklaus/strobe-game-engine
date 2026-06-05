#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include <stdexcept>

namespace strobe::ecs {

struct submit_hook {
  virtual void submit(Scheduler *) = 0;
  virtual ~submit_hook() = default;
};

class OpSchedule;

struct null_schedule_op_t {};
static constexpr null_schedule_op_t null_schedule_op{};

struct schedule_op {
  friend class OpSchedule;

  schedule_op(null_schedule_op_t) : m_ptr(nullptr) {}

  schedule_op() : m_ptr(nullptr) {}

  friend bool operator==(schedule_op op, null_schedule_op_t) noexcept {
    return op.m_ptr == nullptr;
  }
  friend bool operator==(null_schedule_op_t, schedule_op op) noexcept {
    return op.m_ptr == nullptr;
  }
  friend bool operator!=(schedule_op op, null_schedule_op_t) noexcept {
    return op.m_ptr != nullptr;
  }
  friend bool operator!=(null_schedule_op_t, schedule_op op) noexcept {
    return op.m_ptr != nullptr;
  }

private:
  explicit schedule_op(void *ptr) : m_ptr(ptr) {}
  void *m_ptr = nullptr;
};

class OpSchedule {
private:
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

public:
  explicit OpSchedule(const allocator &alloc)
      : m_nodePool(alloc), m_dependencyPool(alloc), m_executionOrder(alloc) {
    m_executionOrder.reserve(64);
  }

  OpSchedule(const OpSchedule &) = delete;
  OpSchedule &operator=(const OpSchedule &) = delete;
  OpSchedule(OpSchedule &&) = delete;
  OpSchedule &operator=(OpSchedule &&) = delete;

  schedule_op create(submit_hook *hook) noexcept {
    assert(hook != nullptr);
    Node *node = static_cast<Node *>(m_nodePool.allocate());
    std::construct_at(node);
    node->hook = hook;
    node->alive = true;
    node->enabled = false;
    node->next = nullptr;
    node->prev = m_orderTail;
    if (m_orderTail != nullptr) {
      m_orderTail->next = node;
    } else {
      m_orderHead = node;
    }
    m_orderTail = node;
    ++m_nodeCount;
    return schedule_op{node};
  }

  void destroy(schedule_op op) noexcept {
    disable(op);
    Node *node = static_cast<Node *>(op.m_ptr);
    assert(node != nullptr);
    assert(node->alive);
    node->alive = false;
    node->hook = nullptr;
    assert(m_nodeCount > 0);
    --m_nodeCount;
    m_dirty = true;
  }

  void enable(schedule_op op) noexcept {
    Node *node = static_cast<Node *>(op.m_ptr);
    assert(node != nullptr);
    assert(node->alive);
    if (!node->enabled) {
      node->enabled = true;
      m_dirtyExecution = true;
    }
  }
  void disable(schedule_op op) noexcept {
    Node *node = static_cast<Node *>(op.m_ptr);
    assert(node != nullptr);
    assert(node->alive);
    if (node->enabled) {
      node->enabled = false;
      m_dirtyExecution = true;
    }
  }

  void sequenced_after(schedule_op before, schedule_op after) noexcept {
    Node *before_node = static_cast<Node *>(before.m_ptr);
    Node *after_node = static_cast<Node *>(after.m_ptr);
    assert(before_node != nullptr);
    assert(after_node != nullptr);
    assert(before_node->alive);
    assert(after_node->alive);
    Dependency *dep = static_cast<Dependency *>(m_dependencyPool.allocate());
    std::construct_at(dep);
    dep->after = after_node;
    dep->next = before_node->successors;
    before_node->successors = dep;
    m_dirty = true;
  }

  void submit_all(Scheduler *scheduler) noexcept {
    if (m_dirty) {
      rebuild_order();
    }
    if (m_dirtyExecution) {
      rebuild_execution_order();
    }
    for (submit_hook *system : m_executionOrder) {
      system->submit(scheduler);
    }
  }

private:
  static constexpr uint32_t unvisited = 0;
  static constexpr uint32_t visiting = 1;
  static constexpr uint32_t visited = 2;

  struct Node;

  struct Dependency {
    Dependency *next = nullptr;
    Node *after = nullptr;
  };

  struct Node {
    Node *next = nullptr;
    Node *prev = nullptr;
    Node *rebuild_next = nullptr;
    submit_hook *hook = nullptr;

    Dependency *successors = nullptr;
    uint32_t rebuild_state : 8 = unvisited;
    uint32_t alive : 1 = false;
    uint32_t enabled : 1;
  };

private:
  void destroy_dependency(Dependency *dep) noexcept {
    assert(dep != nullptr);
    std::destroy_at(dep);
    m_dependencyPool.deallocate(static_cast<void *>(dep));
  }

  void clear_successors(Node *node) noexcept {
    assert(node != nullptr);
    Dependency *dep = node->successors;
    while (dep != nullptr) {
      Dependency *next = dep->next;
      destroy_dependency(dep);
      dep = next;
    }
    node->successors = nullptr;
  }

  void dfs_visit(Node *node, Node *&newHead, Node *&newTail,
                 uint32_t &emitted) noexcept {
    assert(node != nullptr);
    assert(node->alive);
    assert(node->rebuild_state == unvisited);
    node->rebuild_state = visiting;
    Dependency **link = &node->successors;
    while (*link != nullptr) {
      Dependency *dep = *link;
      Node *after = dep->after;
      assert(after != nullptr);
      if (!after->alive) {
        *link = dep->next;
        destroy_dependency(dep);
        continue;
      }
      if (after->rebuild_state == visiting) {
        assert(false && "OpSchedule contains a cycle");
      } else if (after->rebuild_state == unvisited) {
        dfs_visit(after, newHead, newTail, emitted);
      }
      link = &dep->next;
    }
    node->rebuild_state = visited;

    // Prepend to new topological order.
    node->prev = nullptr;
    node->next = newHead;
    if (newHead != nullptr) {
      newHead->prev = node;
    } else {
      newTail = node;
    }
    newHead = node;
    ++emitted;
  }

  void rebuild_order() noexcept {
    Node *oldHead = m_orderHead;
    Node *liveHead = nullptr;
    Node *liveTail = nullptr;
    Node *reclaimHead = nullptr;

    for (Node *node = oldHead; node != nullptr;) {
      Node *next = node->next;
      node->next = nullptr;
      node->prev = nullptr;
      node->rebuild_next = nullptr;
      if (node->alive) {
        node->rebuild_state = unvisited;
        if (liveTail != nullptr) {
          liveTail->rebuild_next = node;
        } else {
          liveHead = node;
        }
        liveTail = node;
      } else {
        clear_successors(node);
        node->next = reclaimHead;
        reclaimHead = node;
      }
      node = next;
    }

    Node *newHead = nullptr;
    Node *newTail = nullptr;
    uint32_t emitted = 0;
    for (Node *node = liveHead; node != nullptr;) {
      Node *next = node->rebuild_next;
      if (node->rebuild_state == unvisited) {
        dfs_visit(node, newHead, newTail, emitted);
      }
      node->rebuild_next = nullptr;
      node = next;
    }

    assert(emitted == m_nodeCount);
    m_orderHead = newHead;
    m_orderTail = newTail;
    for (Node *node = reclaimHead; node != nullptr;) {
      Node *next = node->next;
      std::destroy_at(node);
      m_nodePool.deallocate(static_cast<void *>(node));
      node = next;
    }
    m_dirty = false;
    m_dirtyExecution = true;
  }

  void rebuild_execution_order() noexcept {
    m_executionOrder.clear();
    for (Node *node = m_orderHead; node != nullptr; node = node->next) {
      assert(node->alive);
      if (node->enabled) {
        assert(node->hook != nullptr);
        m_executionOrder.push_back(node->hook);
      }
    }
    m_dirtyExecution = false;
  }

private:
  template <typename T>
  using Pool = MonotonicPoolResource<sizeof(T), alignof(T), allocator>;

  Pool<Node> m_nodePool;
  Pool<Dependency> m_dependencyPool;

  Vector<submit_hook *, allocator> m_executionOrder;

  Node *m_orderHead = nullptr;
  Node *m_orderTail = nullptr;

  uint32_t m_nodeCount = 0;

  bool m_dirty : 1 = true;
  bool m_dirtyExecution : 1 = true;
};

} // namespace strobe::ecs
