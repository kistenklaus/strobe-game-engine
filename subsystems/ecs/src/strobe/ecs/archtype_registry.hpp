#pragma once

#include "strobe/core/containers/small_vector_storage.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/core/containers/vector_storage.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/component_type.hpp"
#include <algorithm>
#include <cstdint>

namespace strobe::ecs {

using archetype_id = uint32_t;
using column = uint32_t;

class ArchetypeTable {
  
};

class ArchetypeRegistry {
private:
  using allocator = strobe::ecs::allocator_ref;
  struct ArchetypeRecord {
    SmallVectorStorage<component_id, 8> components;
    // TODO: add small cache for add / remove transitions.
    ArchetypeTable table;
  };

public:
  ArchetypeRegistry(const allocator &alloc) : m_alloc(alloc) {}

  /// Expects sorted, unique component ids.
  archetype_id find_or_create(span<const component_id> components) {
    assert(std::ranges::is_sorted(components.begin(), components.end()));
    assert(std::adjacent_find(components.begin(), components.end()) ==
           components.end());

    const archetype_id n = static_cast<archetype_id>(m_archetypes.size());
    for (archetype_id id = 0; id < n; ++id) {
      if (std::ranges::equal(m_archetypes[id].components, components)) {
        return id;
      }
    }
    const archetype_id id = static_cast<archetype_id>(m_archetypes.size());
    ArchetypeRecord &archetype = m_archetypes.emplace_back(m_alloc);
    archetype.components.assign(m_alloc, components.begin(), components.end());

    return id;
  }

  std::pair<archetype_id, column> add_transition(archetype_id src,
                                                 component_id cid) {
    assert(src < m_archetypes.size());

    span<const component_id> src_components = components(src);

    // Check whether the target archetype already exists:
    // target == src_components with cid inserted.
    const archetype_id n = static_cast<archetype_id>(m_archetypes.size());
    for (archetype_id id = 0; id < n; ++id) {
      span<const component_id> candidate = components(id);
      if (candidate.size() != src_components.size() + 1) {
        continue;
      }

      size_t src_i = 0;
      column added_col = 0;
      bool consumed_added = false;
      bool matches = true;

      for (column candidate_col = 0;
           candidate_col < static_cast<column>(candidate.size());
           ++candidate_col) {
        const component_id c = candidate[candidate_col];

        if (!consumed_added && c == cid) {
          consumed_added = true;
          added_col = candidate_col;
          continue;
        }

        if (src_i >= src_components.size() || src_components[src_i] != c) {
          matches = false;
          break;
        }

        ++src_i;
      }

      if (matches && consumed_added && src_i == src_components.size()) {
        return {id, added_col};
      }
    }

    // Create sorted component list.
    ArchetypeRecord &archetype = m_archetypes.emplace_back(m_alloc);
    archetype.components.reserve(m_alloc, src_components.size() + 1);

    column added_col = 0;
    bool inserted = false;

    for (component_id c : src_components) {
      assert(c != cid && "cannot add component that already exists");

      if (!inserted && cid < c) {
        added_col = static_cast<column>(archetype.components.size());
        archetype.components.emplace_back(m_alloc, cid);
        inserted = true;
      }

      archetype.components.emplace_back(m_alloc, c);
    }

    if (!inserted) {
      added_col = static_cast<column>(archetype.components.size());
      archetype.components.emplace_back(m_alloc, cid);
    }

    return {
        static_cast<archetype_id>(m_archetypes.size() - 1),
        added_col,
    };
  }

  std::pair<archetype_id, column> remove_transition(archetype_id src,
                                                    component_id cid) {
    assert(src < m_archetypes.size());

    span<const component_id> src_components = components(src);

    // Check whether the target archetype already exists:
    // target == src_components with cid removed.
    const archetype_id n = static_cast<archetype_id>(m_archetypes.size());
    for (archetype_id id = 0; id < n; ++id) {
      span<const component_id> candidate = components(id);
      if (candidate.size() + 1 != src_components.size()) {
        continue;
      }

      size_t candidate_i = 0;
      column removed_col = 0;
      bool consumed_removed = false;
      bool matches = true;

      for (column src_col = 0;
           src_col < static_cast<column>(src_components.size()); ++src_col) {
        const component_id c = src_components[src_col];

        if (!consumed_removed && c == cid) {
          consumed_removed = true;
          removed_col = src_col;
          continue;
        }

        if (candidate_i >= candidate.size() || candidate[candidate_i] != c) {
          matches = false;
          break;
        }

        ++candidate_i;
      }

      if (matches && consumed_removed && candidate_i == candidate.size()) {
        return {id, removed_col};
      }
    }

    // Create persistent sorted component list.
    ArchetypeRecord &archetype = m_archetypes.emplace_back(m_alloc);
    archetype.components.reserve(m_alloc, src_components.size() - 1);

    column removed_col = 0;
    bool removed = false;

    for (column src_col = 0;
         src_col < static_cast<column>(src_components.size()); ++src_col) {
      const component_id c = src_components[src_col];

      if (!removed && c == cid) {
        removed = true;
        removed_col = src_col;
        continue;
      }

      archetype.components.emplace_back(m_alloc, c);
    }

    assert(removed && "cannot remove component that does not exist");

    return {
        static_cast<archetype_id>(m_archetypes.size() - 1),
        removed_col,
    };
  }

  span<const component_id> components(archetype_id id) const {
    assert(id < m_archetypes.size());
    return m_archetypes[id].components;
  }

  bool contains(archetype_id id, component_id cid) const {
    assert(id < m_archetypes.size());
    span<const component_id> comps = components(id);
    return std::ranges::binary_search(comps, cid);
  }

  uint32_t index(archetype_id id, component_id cid) const {
    assert(id < m_archetypes.size());

    span<const component_id> comps = components(id);
    auto it = std::ranges::lower_bound(comps, cid);
    assert(it != comps.end() && *it == cid &&
           "component not contained in archetype");

    return static_cast<uint32_t>(it - comps.begin());
  }

private:
  allocator m_alloc;
  VectorStorage<ArchetypeRecord> m_archetypes;
};

} // namespace strobe::ecs
