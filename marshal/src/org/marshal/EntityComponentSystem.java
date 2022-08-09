package org.marshal;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;
import org.marshal.intern.ComponentType;
import org.marshal.intern.ComponentTypeHandler;
import org.marshal.systems.EntitySystem;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Queue;

/**
 * The EntityComponentSystem is the core class of the Marshal library
 * An EntityComponentSystem way of structuring entities in a system
 * Entities are simple objects that can have components assigned to them
 * We Iterate over Entities using Families (groups of entities) and we
 * do your logic in EntitySystems.
 * <p>
 * The Marshal EntityComponentSystem is not synchronized, because every thread that collides
 * with the ecs gets its own space, for example an family created on one thread is not
 * accessible from a different one.
 * Systems, Families, Events are separated by thread.
 * This is not very space efficient, but it increases performance because this way all systems, events
 * and all families are synchronized without any thread waiting.
 * !entities are obviously not separated by thread, what can create synchronization problems
 * <p>
 * any changes done to the ecs are not directly applied, they are all queued until the ecs is updated.
 * this way there are really no synchronization errors.
 *
 * @author Karl
 * @version 3.0
 */
public final class EntityComponentSystem {

    private final ArrayList<Entity> activeEntities = new ArrayList<>();
    private final Queue<Entity> inactiveEntities = new LinkedList<>();

    private final HashMap<Thread, ThreadHandler> threadHandlers
            = new HashMap<>();
    private final ComponentTypeHandler componentTypeHandler
            = new ComponentTypeHandler();

    /**
     * creates a new EntityComponentSystem
     */
    public EntityComponentSystem() {

    }

    /**
     * registers a Thread
     *
     * @return
     */
    public ThreadHandler registerThread() {
        return registerThread(Thread.currentThread());
    }

    public ThreadHandler registerThread(Thread thread) {

        ThreadHandler threadHandler = threadHandlers.get(thread);
        if(threadHandler!=null) return threadHandler;
        synchronized (threadHandlers){
            threadHandler = new ThreadHandler(activeEntities, this);
            threadHandlers.put(thread, threadHandler);
            return threadHandler;
        }
    }

    public Family getFamily(Class<? extends Component>... componentClasses) {
        return getFamily(Thread.currentThread(), componentClasses);
    }

    public Family getFamily(Thread thread, Class<? extends Component>... componentClasses) {
        ThreadHandler threadHandler = registerThread(thread);
        return threadHandler.registerFamily(componentClasses);
    }

    public void startSystems() {
        ThreadHandler threadHandler = registerThread();
        threadHandler.startSystems();
    }

    public void update() {
        ThreadHandler threadHandler = registerThread();
        threadHandler.update();
    }

    public synchronized Entity createEntity() {
        if (inactiveEntities.isEmpty()) return new Entity(this);
        return inactiveEntities.poll();
    }

    public synchronized void addEntity(Entity entity) {
        if (!entity.isPartOfEcs(this))
            throw new IllegalArgumentException("The entity was not created at this ECS, \n" +
                    "Entity Component Systems can't share entities");
        synchronized (activeEntities) {

            this.activeEntities.add(entity);
            entity.activate();

            synchronized (threadHandlers){
                for (ThreadHandler threadHandler : threadHandlers.values()) {
                    threadHandler.dispatchEntityAdded(entity);
                }
            }
        }
    }

    public synchronized void removeEntity(Entity entity) {
        if (!entity.isPartOfEcs(this)) return;
        synchronized (activeEntities) {

            //O(n) inefficiency
            boolean removed = this.activeEntities.remove(entity);
            entity.deactivate();
            if (!removed) return;

            //Not disposing the Entity

            synchronized (threadHandlers){
                for (ThreadHandler threadHandler : threadHandlers.values()) {
                    threadHandler.dispatchEntityRemoved(entity);
                }
            }
        }
    }

    public synchronized void deleteEntity(Entity entity) {
        removeEntity(entity);
        entity.dispose();
    }

    public void addSystem(EntitySystem system) {
        addSystem(Thread.currentThread(), system);
    }

    public void addSystem(Thread thread, EntitySystem system) {
        ThreadHandler threadHandler = registerThread(thread);
        threadHandler.addSystem(system);
    }

    public void removeSystem(EntitySystem system) {
        removeSystem(Thread.currentThread(), system);
    }

    public void removeSystem(Thread thread, EntitySystem system) {
        ThreadHandler threadHandler = registerThread(thread);
        threadHandler.removeSystem(system);
    }

    public void addListener(EventListener<? extends Event> listener) {
        addListener(Thread.currentThread(), listener);
    }

    public void addListener(Thread thread, EventListener<? extends Event> listener) {
        ThreadHandler threadHandler = registerThread(thread);
        threadHandler.addListener(listener);
    }

    void dispatchComponentAdded(Entity entity, Component component) {
        synchronized (threadHandlers){
            for (ThreadHandler threadHandler : threadHandlers.values()) {
                threadHandler.dispatchComponentAdded(entity, component);
            }
        }
    }

    void dispatchComponentRemoved(Entity entity, Component component) {
        synchronized (threadHandlers){
            for (ThreadHandler threadHandler : threadHandlers.values()) {
                threadHandler.dispatchComponentRemoved(entity, component);
            }
        }
    }

    ComponentType registerComponentType(Class<? extends Component> componentClass) {
        return componentTypeHandler.getFor(componentClass);
    }

    ComponentTypeHandler getComponentTypeHandler() {
        return this.componentTypeHandler;
    }

    void collectEntity(Entity entity) {
        inactiveEntities.add(entity);
    }

}
