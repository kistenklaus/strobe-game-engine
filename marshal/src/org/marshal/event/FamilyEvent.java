package org.marshal.event;

import org.marshal.Entity;
import org.marshal.Family;
import org.marshal.event.intern.Event;


/**
 * An Event that dispatches on Family changes
 * @version 3.0
 * @author Karl
 */
public class FamilyEvent extends Event {

    public static final int FAMILY_MEMBER_ADDED = 0b1;
    public static final int FAMILY_MEMBER_REMOVED = 0b10;

    private final Family family;
    private final Entity member;

    public FamilyEvent(Family family, Entity member, int mods) {
        super(Event.FAMILY_EVENT, mods);
        this.family = family;
        this.member = member;
    }

    public Family getFamily(){
        return family;
    }

    public Entity getMember(){
        return member;
    }
}
