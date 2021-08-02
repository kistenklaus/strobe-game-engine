package org.strobe.ecs;


import java.util.*;
import java.util.function.Consumer;

public final class EntityPool implements Iterable<Entity> {

    private final EntityComponentSystem ecs;
    private final int poolIndex;
    private final Set<Integer> entityIndices = new HashSet<>();
    private final EntityFilter filter;

    private final Set<Entity> scheduledForUpdates = new HashSet<>();

    private final Map<Entity, PoolEvent> scheduledEvents = new HashMap<>();
    private final Collection<Consumer<Entity>> entityAddedObservers = new ArrayList<>();
    private final Collection<Consumer<Entity>> entityRemovedObservers = new ArrayList<>();

    protected EntityPool(EntityComponentSystem ecs, EntityFilter filter, int poolIndex) {
        this.ecs = ecs;
        this.filter = filter;
        this.poolIndex = poolIndex;
    }

    protected void updatePool() {
        for (Entity entity : scheduledForUpdates) {
            boolean current = contains(entity);
            boolean desired = filter.eval(entity);
            if (current && !desired) {
                entity.removeFromPoolInternal(EntityPool.this);
                EntityPool.this.removeEntityInternal(entity);
                enqueueEntityRemovedEvent(entity);
            } else if (!current && desired) {
                entity.addToPoolInternal(EntityPool.this);
                EntityPool.this.addEntityInternal(entity);
                enqueueEntityAddedEvent(entity);
            }
        }
        scheduledForUpdates.clear();
    }

    protected void processEvents() {
        for (PoolEvent event : scheduledEvents.values()) {
            event.execute();
        }
        scheduledEvents.clear();
    }

    protected void enqueueEntityComponentChange(Entity entity, ComponentType componentType) {
        if (!filter.isSensitive(componentType)) return;
        enqueueEntityUpdate(entity);
    }

    protected void enqueueEntityUpdate(Entity entity) {
        scheduledForUpdates.add(entity);
    }

    public boolean contains(Entity entity) {
        return entityIndices.contains(entity.getEntityIndex());
    }

    private void addEntityInternal(Entity entity) {
        entityIndices.add(entity.getEntityIndex());
    }

    private void removeEntityInternal(Entity entity) {
        entityIndices.remove(entity.getEntityIndex());
    }

    private void enqueueEntityAddedEvent(Entity entity) {
        PoolEvent event = new PoolEvent(entity, entityAddedObservers);
        scheduledEvents.put(entity, event);
    }

    private void enqueueEntityRemovedEvent(Entity entity) {
        PoolEvent event = new PoolEvent(entity, entityRemovedObservers);
        scheduledEvents.put(entity, event);
    }

    public EntityPoolObserver addEntityAddedObserver(EntityPoolObserver observer){
        entityAddedObservers.add(observer);
        return observer;
    }

    public void removeEntityAddedObserver(EntityPoolObserver observer){
        entityAddedObservers.remove(observer);
    }

    public EntityPoolObserver addEntityRemovedObserver(EntityPoolObserver observer){
        entityRemovedObservers.add(observer);
        return observer;
    }

    public void removeEntityRemovedObserver(EntityPoolObserver observers){
        entityRemovedObservers.remove(observers);
    }

    protected int getPoolIndex() {
        return poolIndex;
    }

    public int getSize() {
        return entityIndices.size();
    }

    @Override
    public Iterator<Entity> iterator() {
        return new Iterator<>() {

            private final Iterator<Integer> indices = entityIndices.iterator();

            @Override
            public boolean hasNext() {
                return indices.hasNext();
            }

            @Override
            public Entity next() {
                return ecs.getEntity(indices.next());
            }
        };
    }

    public boolean isEmpty() {
        return entityIndices.isEmpty();
    }

    public Entity getAny() {
        if(isEmpty())return null;
        return ecs.getEntity(entityIndices.iterator().next());
    }

    private static final class PoolEvent {
        private final Entity entity;
        private final Collection<Consumer<Entity>> observers;

        public PoolEvent(Entity entity, Collection<Consumer<Entity>> observers) {
            this.entity = entity;
            this.observers = observers;
        }

        protected void execute() {
            for (Consumer<Entity> observer : observers) {
                observer.accept(entity);
            }
        }
    }
}
