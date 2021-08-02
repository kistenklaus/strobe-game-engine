package org.strobe.ecs.context.renderer.transform;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.EntityFilter;
import org.strobe.ecs.EntityPool;
import org.strobe.ecs.EntitySystem;
import org.strobe.gfx.transform.AbstractTransform;

public final class TransformSystem extends EntitySystem {

    public TransformSystem(EntityComponentSystem ecs) {
        super(ecs);
    }

    @Override
    public void init() {}

    public void update(float dt) {
        AbstractTransform.updateTransforms();
    }

    @Override
    public void dispose() {}
}
