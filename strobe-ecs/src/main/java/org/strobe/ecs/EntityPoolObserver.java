package org.strobe.ecs;

import java.util.function.Consumer;

@FunctionalInterface
public interface EntityPoolObserver extends Consumer<Entity> {
}
