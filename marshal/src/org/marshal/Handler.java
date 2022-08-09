package org.marshal;

import java.util.Objects;

/**
 * A Handler is handling an entity.
 * its used to check the phase difference between entities.
 * an entity that's phase has changed should not be accessible throw the handler.
 *
 * @author Karl
 * @version 3.0
 */
class Handler {

    private final int phase;

    private final Entity entity;

    public Handler(Entity entity) {
        this.entity = entity;
        this.phase = entity.getPhase();
    }

    public boolean isCurrent() {
        return this.phase == entity.getPhase();
    }

    public Entity get() {
        if (isCurrent()) return entity;
        else return null;
    }

    public Entity getEntity() {
        return this.entity;
    }

    public int hashCode() {
        return Objects.hash(Integer.valueOf(phase), entity);
    }


}
