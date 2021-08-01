package org.strobe.ecs;

import java.util.function.BiConsumer;

@FunctionalInterface
public interface ComponentObserver extends BiConsumer<Entity, Class<? extends Component>> {

}
