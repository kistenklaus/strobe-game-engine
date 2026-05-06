#pragma once

#include "strobe/ecs/entity.hpp"
namespace strobe {

class EntityCommands {
public:
  Entity createEntity();
  template <typename... Ts> Entity createEntity(Ts &&...components);
  void destroyEntity(Entity entity);

  template <typename T> void addComponent(Entity e, T &&component);
  template <typename T> void removeComponent(Entity e);

private:
};

class ResourceCommands {
public:
  template <typename R> void createResource(R &&resource);
  template <typename R> void destroyResource();

private:
};

class SystemCommands {
public:
  template <typename S> bool registerSystem(S &&system);
  template <typename S> bool unregisterSystem();

  template <typename S> bool enableSystem();
  template <typename S> bool disableSystem();

private:
};

class TaskCommands {
public:
  template <typename T> void submit(T &&task);
private:
};

} // namespace strobe
