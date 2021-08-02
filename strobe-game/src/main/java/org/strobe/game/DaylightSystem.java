package org.strobe.game;

import org.joml.Vector3f;
import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.transform.Transform;

public final class DaylightSystem extends EntitySystem {

    private final EntityPool pool;

    private float cycle = 0;

    public DaylightSystem(EntityComponentSystem ecs) {
        super(ecs);

        pool = ecs.createPool(EntityFilter.requireAll(EntityFilter.requireAll(DaylightCycle.class, Transform.class)));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        cycle += dt * 0.25f;
        for(Entity entity : pool){
            Transform transform = entity.get(Transform.class);
            transform.setPosition(0, (float) Math.cos(cycle)*3,
                    (float)Math.sin(cycle)*3);
        }
    }

    @Override
    public void dispose() {

    }
}
