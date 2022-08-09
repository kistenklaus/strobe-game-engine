package org.marshal;

import org.marshal.intern.ComponentMask;
import org.marshal.intern.ComponentType;

import java.util.ArrayList;
import java.util.HashMap;


/**
 * handles all families that are current on the thread that the family Handler is attched to
 *
 * @author Karl
 * @version 3.0
 */
class FamilyHandler {

    private final ArrayList<Entity> ecsEntities;
    private final EntityComponentSystem ecs;
    private final HashMap<String, Family> familiesMap = new HashMap<>();
    private EventHandler eventHandler;

    public FamilyHandler(EventHandler eventHandler, ArrayList<Entity> ecsEntities,
                         EntityComponentSystem ecs) {
        this.eventHandler = eventHandler;
        this.ecsEntities = ecsEntities;
        this.ecs = ecs;
    }

    public Family registerFamily(Class<? extends Component>... familyClasses) {
        ComponentMask familyMask = new ComponentMask();
        for (Class<? extends Component> componentClass : familyClasses) {
            ComponentType type = ecs.registerComponentType(componentClass);
            familyMask.set(type.getIndex(), true);
        }

        String familyString = familyMask.toString();
        return familiesMap.computeIfAbsent(familyString,
                s -> createFamily(familyClasses));
    }

    private Family createFamily(Class<? extends Component>... familyClasses) {
        ComponentMask[] familyMask = new ComponentMask[familyClasses.length];
        for (int i = 0; i < familyClasses.length; i++) {
            Class<? extends Component> componentClass = familyClasses[i];
            ComponentType type = ecs.registerComponentType(componentClass);
            familyMask[i] = type.getAssignableComponents();
        }

        Family family = new Family(familyMask, eventHandler);

        synchronized (ecsEntities) {
            for (Entity entity : ecsEntities) {
                family.updateMembership(entity);
            }
        }

        return family;
    }

    public void addNewEntity(Entity entity) {
        for (Family family : familiesMap.values()) {
            family.updateMembership(entity);
        }
    }

    public void updateFamilyMember(Entity entity) {
        for (Family family : familiesMap.values()) {
            family.updateMembership(entity);
        }
    }

    public void removeOldEntity(Entity entity) {
        for (Family family : familiesMap.values()) {
            family.updateMembership(entity);
        }
    }
}
