package org.marshal;

import org.marshal.event.FamilyEvent;
import org.marshal.intern.ComponentMask;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * A family is a collection of entities in the ecs that match the requirements of
 * the family. An Family is defined by a set of component classes. For an entity to be
 * included in the family is needs to have one component that is assignable from the component class for
 * each component class.
 *
 * <br>
 * For Example:<br>
 * Entity : [ComponentB]<br>
 * ComponentB inherits ComponentA<br>
 * than the family [ComponentA] would include the entity<br>
 * <br>
 *
 * An Family is Iterable!!
 *
 * @version 3.0
 * @author Karl
 */
public final class Family implements Iterable<Entity> {

    private final Vector<Handler> members = new Vector<>();
    private final HashMap<Entity, Handler> memberMap = new HashMap<>();

    private final ComponentMask[] familyMask;

    private final EventHandler eventHandler;

    private final FamilyIterator familyIterator;

    /**
     * creates a new Family
     * @param familyMask the familyMask
     * @param eventHandler the eventHandler of the Thread the Family is in.
     */
    public Family(ComponentMask[] familyMask, EventHandler eventHandler) {
        this.familyMask = familyMask;
        this.eventHandler = eventHandler;
        this.familyIterator = new FamilyIterator();
    }

    /**
     * updates the membership of one entity
     * @param entity the entity to update the membership
     */
    public void updateMembership(Entity entity) {

        boolean isMember = memberMap.containsKey(entity);

        if (isMember) {
            if (!memberMap.get(entity).isCurrent() || !entity.isActive()) {
                exile(entity);
                return;
            }
        }else if(!entity.isActive()){
            return;
        }

        boolean matches = matches(entity);

        if (matches && !isMember) {
            takeIn(entity);
        } else if (!matches && isMember) {
            exile(entity);
        }
    }

    /**
     * @param entity the entity to check
     * @return true if the entity matches the family requirements
     */
    boolean matches(Entity entity) {
        for (ComponentMask mask : familyMask) {
            if (!mask.hasOneOf(entity.getComponentMask())) return false;
        }
        return true;
    }

    /**
     * @param entity includes a entity into the family
     */
    public void takeIn(Entity entity) {
        Handler handler = new Handler(entity);
        members.add(handler);
        memberMap.put(entity, handler);
        eventHandler.queueEvent(
                new FamilyEvent(this, entity, FamilyEvent.FAMILY_MEMBER_ADDED));
    }

    /**
     * exiles an entity from the family
     * @param entity the entity to exile from the family
     */
    public void exile(Entity entity) {
        //O(n) inefficiency
        for (int i = 0; i < members.size(); i++) {
            if (entity.equals(members.get(i).getEntity())) {
                members.remove(i);
                break;
            }
        }

        memberMap.remove(entity);
        eventHandler.queueEvent(
                new FamilyEvent(this, entity, FamilyEvent.FAMILY_MEMBER_REMOVED));
    }

    public int size(){
        return members.size();
    }


    private class FamilyIterator implements Iterator<Entity> {

        private int it = 0;

        private Entity next;


        public void reset(){
            it = 0;
            next = null;
        }

        @Override
        public boolean hasNext() {
            next = findNext();
            return next != null;
        }

        private Entity findNext() {
            while(it < members.size()){
                if (members.get(it).isCurrent()) {
                    return members.get(it++).getEntity();
                }
                it++;
            }
            return null;
        }

        @Override
        public Entity next() {
            return next;
        }
    }

    /**
     * creates an iterator to iterate over the family
     */
     @Override
    public Iterator<Entity> iterator() {
         familyIterator.reset();
        return familyIterator;
    }

    /**
     * @return a string representing an entity array
     */
    public String toString(){
        StringBuilder str = new StringBuilder();
        str.append("[");
        for(Entity entity : this){
            str.append(entity).append(", ");
        }
        if(!members.isEmpty()){
            str.delete(str.length()-2, str.length());
        }
        str.append("]");
        return str.toString();
    }
}
