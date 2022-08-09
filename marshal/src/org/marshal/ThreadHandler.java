package org.marshal;

import org.marshal.event.*;
import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;
import org.marshal.systems.EntitySystem;

import java.util.ArrayList;


/**
 * the thread handler is handling everything that is attached to one thread.
 * systems, events, families.
 * the thread handler does not handle the entities
 *
 * @author Karl
 * @version 3.0
 */
class ThreadHandler {


    private final SystemHandler systemHandler;
    private final EventHandler eventHandler = new EventHandler();
    private final FamilyHandler familyHandler;

    public ThreadHandler(ArrayList<Entity> ecsEntities, EntityComponentSystem ecs) {
        eventHandler.addListener(new EntityCallback());
        eventHandler.addListener(new ComponentCallback());
        eventHandler.addListener(new SystemCallback());
        familyHandler = new FamilyHandler(eventHandler, ecsEntities, ecs);
        systemHandler = new SystemHandler(ecs);
    }

    public void startSystems() {
        systemHandler.startSystems();
    }

    public void update() {
        eventHandler.pollEvents();
        systemHandler.updateSystems();
    }

    @SafeVarargs
    public final Family registerFamily(Class<? extends Component>... familyClasses) {
        return familyHandler.registerFamily(familyClasses);
    }

    public void dispatchEntityAdded(Entity entity) {
        eventHandler.queueEvent(
                new EntityEvent(entity, EntityEvent.ENTITY_ADDED));
    }

    public void dispatchEntityRemoved(Entity entity) {
        eventHandler.queueEvent(
                new EntityEvent(entity, EntityEvent.ENTITY_REMOVED));
    }

    public void dispatchComponentAdded(Entity entity, Component component) {
        eventHandler.queueEvent(
                new ComponentEvent(entity, component, ComponentEvent.COMPONENT_ADDED));
    }

    public void dispatchComponentRemoved(Entity entity, Component component) {
        eventHandler.queueEvent(
                new ComponentEvent(entity, component, ComponentEvent.COMPONENT_REMOVED));
    }

    public void addSystem(EntitySystem system) {
        if (!systemHandler.containsSystem(system))
            eventHandler.queueEvent(
                    new SystemEvent(system, SystemEvent.SYSTEM_ADDED));
    }

    public void removeSystem(EntitySystem system) {
        eventHandler.queueEvent(
                new SystemEvent(system, SystemEvent.SYSTEM_REMOVED));
    }

    public void addListener(EventListener<? extends Event> listener) {
        eventHandler.addListener(listener);
    }

    private class EntityCallback extends EntityListener {
        @Override
        public void dispatched(EntityEvent entityEvent) {
            if (entityEvent.isMod(EntityEvent.ENTITY_ADDED)) {
                familyHandler.addNewEntity(entityEvent.getEntity());
            } else if (entityEvent.isMod(EntityEvent.ENTITY_REMOVED)) {
                familyHandler.removeOldEntity(entityEvent.getEntity());
            }
        }
    }

    private class ComponentCallback extends ComponentListener {
        @Override
        public void dispatched(ComponentEvent componentEvent) {
            if (componentEvent.isMod(ComponentEvent.COMPONENT_ADDED)) {
                familyHandler.updateFamilyMember(componentEvent.getEntity());
            } else if (componentEvent.isMod(ComponentEvent.COMPONENT_REMOVED)) {
                familyHandler.updateFamilyMember(componentEvent.getEntity());
            }
        }
    }

    private class SystemCallback extends SystemListener {
        @Override
        public void dispatched(SystemEvent systemEvent) {
            if (systemEvent.isMod(SystemEvent.SYSTEM_ADDED)) {
                systemHandler.addSystem(systemEvent.getSystem());
            } else if (systemEvent.isMod(SystemEvent.SYSTEM_REMOVED)) {
                systemHandler.removeSystem(systemEvent.getSystem());
            }
        }
    }
}
