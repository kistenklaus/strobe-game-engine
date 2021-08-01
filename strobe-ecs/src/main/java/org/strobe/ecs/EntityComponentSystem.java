package org.strobe.ecs;


import org.strobe.utils.UnboundedArray;

import java.util.*;
import java.util.function.BiConsumer;

public class EntityComponentSystem {

    private final Stack<Entity> deadEntities = new Stack<>();
    private final UnboundedArray<Entity> entities = new UnboundedArray<>(8);

    private final UnboundedArray<EntityPool> entityPools = new UnboundedArray<>(8);
    private final Map<EntityFilter, Integer> poolMap = new HashMap<>();
    private final Stack<EntityPool> scheduledForFullUpdate = new Stack<>();

    private final LinkedList<Runnable> entityOperationQueue = new LinkedList<>();
    private final LinkedList<Runnable> systemOperationQueue = new LinkedList<>();

    private final LinkedList<Runnable> entityComponentEvents = new LinkedList<>();

    private boolean instantUpdateOutOfCycle = false;
    private boolean inCycle = false;

    private final ArrayList<EntitySystem> systems = new ArrayList<>();

    public EntityComponentSystem() {
    }

    public Entity createEntity() {
        if (!deadEntities.isEmpty()) {
            Entity entity = deadEntities.pop();
            return entity;
        } else {
            int entityIndex = entities.getNextFreeIndex();
            Entity entity = new Entity(this);
            entity.respawn(entityIndex);
            entities.set(entityIndex, entity);
            return entity;
        }
    }

    public EntityPool createPool(EntityFilter filter) {
        Integer poolIndex = poolMap.get(filter);
        if (poolIndex != null) return entityPools.get(poolIndex);
        poolIndex = entityPools.getNextFreeIndex();
        EntityPool pool = new EntityPool(this, filter, poolIndex);
        entityPools.add(pool);
        poolMap.put(filter, poolIndex);
        scheduledForFullUpdate.push(pool);
        return pool;
    }

    public void addEntitySystem(EntitySystem entitySystem){
        systems.add(entitySystem);
        systemOperationQueue.add(entitySystem::init);
        if(instantUpdateOutOfCycle && !inCycle)processSystemOperations();
    }

    public void removeEntitySystem(EntitySystem entitySystem){
        systems.remove(entitySystem);
        systemOperationQueue.add(entitySystem::dispose);
        if(instantUpdateOutOfCycle && !inCycle)processSystemOperations();
    }

    public void update(float dt) {
        inCycle = true;

        Phase.BEGIN.execute(this, dt);

        inCycle = false;
    }

    private void processSystemOperations(){
        while(!systemOperationQueue.isEmpty()){
            systemOperationQueue.pop().run();
        }
    }

    private void executeSystems(float dt) {
        //TODO concurrent execution
        for (EntitySystem entitySystem : systems) {
            //TODO disabled systems could be optimized
            if(entitySystem.isEnabled())entitySystem.update(dt);
        }
    }

    private void processEntityOperations() {
        while (!entityOperationQueue.isEmpty()){
            entityOperationQueue.pop().run();
        }
    }

    private void processEntityPoolChanges() {
        while(!scheduledForFullUpdate.isEmpty()){
            EntityPool pool = scheduledForFullUpdate.pop();
            for(Entity entity : entities){
                pool.enqueueEntityUpdate(entity);
            }
        }
        //TODO concurrent execution
        for (EntityPool entityPool : entityPools) {
            entityPool.updatePool();
        }
    }

    private void processEntityPoolEvents(){
        //process entity pool events
        for(EntityPool entityPool : entityPools){
            entityPool.processEvents();
        }
    }

    private void processEntityComponentEvents(){
        //process entity component events
        while(!entityComponentEvents.isEmpty()){
            entityComponentEvents.pop().run();
        }
    }

    protected void enqueueEntityOperation(Runnable operation) {
        entityOperationQueue.addLast(operation);
        if (!inCycle && instantUpdateOutOfCycle) {
            inCycle = true;
            Phase.PROCESS_ENTITY_CHANGES.execute(this, null);
            inCycle = false;
        }
    }

    protected void enqueueEntityComponentChange(Entity entity, ComponentType componentType) {
        for (EntityPool entityPool : entityPools) {
            entityPool.enqueueEntityComponentChange(entity, componentType);
        }
        if (!inCycle && instantUpdateOutOfCycle) {
            inCycle = true;
            Phase.PROCESS_ENTITY_POOL_CHANGES.execute(this, null);
            inCycle = false;
        }
    }

    protected void enqueueEntityComponentEvent(Entity entity, ComponentType componentType, boolean added){
        entityComponentEvents.add(()->entity.notifyObservers(componentType, added));
        if(!inCycle && instantUpdateOutOfCycle){
            inCycle = true;
            Phase.PROCESS_ENTITY_COMPONENT_EVENTS.execute(this, null);
            inCycle = false;
        }
    }

    protected Entity getEntity(int entityIndex) {
        return entities.get(entityIndex);
    }

    public void disableInstantUpdate() {
        instantUpdateOutOfCycle = false;
    }

    public void enableInstantUpdate() {
        instantUpdateOutOfCycle = true;
    }

    private enum Phase{
        BEGIN(null),
        PREPARE_SYSTEMS((ecs,dt) -> ecs.processSystemOperations()),
        EXECUTE_SYSTEMS((ecs,dt)-> ecs.executeSystems(dt)),
        PROCESS_ENTITY_CHANGES((ecs, dt)->ecs.processEntityOperations()),
        PROCESS_ENTITY_POOL_CHANGES((ecs, dt)->ecs.processEntityPoolChanges()),
        PROCESS_ENTITY_POOL_EVENTS((ecs, dt)->ecs.processEntityPoolEvents()),
        PROCESS_ENTITY_COMPONENT_EVENTS((ecs, dt)->ecs.processEntityComponentEvents()),
        END(null);


        private BiConsumer<EntityComponentSystem, Float> phaseExecutor;

        Phase(BiConsumer<EntityComponentSystem, Float> phase_function) {
            this.phaseExecutor = phase_function;
        }

        public void execute(EntityComponentSystem ecs, Float dt){
            if(phaseExecutor != null)this.phaseExecutor.accept(ecs, dt);
            if(this.ordinal()+1<Phase.values().length)
                Phase.values()[this.ordinal()+1].execute(ecs,dt);
        }
    }
}
