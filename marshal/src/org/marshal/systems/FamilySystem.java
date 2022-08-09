package org.marshal.systems;

import org.marshal.Entity;
import org.marshal.Family;
import org.marshal.event.FamilyEvent;
import org.marshal.event.FamilyListener;

public abstract class FamilySystem extends IntervalSystem {

    private final Family systemFamily;

    public FamilySystem(Family family) {
        super();
        this.systemFamily = family;
    }

    public FamilySystem(Family family, float interval) {
        super(interval);
        this.systemFamily = family;
    }

    public final void start() {
        getEcs().addListener(new FamilyListener() {
            @Override
            public void dispatched(FamilyEvent familyEvent) {
                if (familyEvent.isMod(FamilyEvent.FAMILY_MEMBER_ADDED)) {
                    entityAdded(familyEvent.getMember());
                } else if (familyEvent.isMod(FamilyEvent.FAMILY_MEMBER_REMOVED)) {
                    entityRemoved(familyEvent.getMember());
                }
            }
        });
        _start();
    }

    public abstract void _start();

    @Override
    public final void interval(float dt) {
        for (Entity entity : systemFamily) {
            updateEntity(entity, dt);
        }
    }

    public abstract void updateEntity(Entity entity, float dt);

    public abstract void stop();

    public abstract void entityAdded(Entity entity);

    public abstract void entityRemoved(Entity entity);

    public Family getSystemFamily(){
        return systemFamily;
    }
}
