package org.strobe.ecs.context.renderer.transform;

import org.strobe.ecs.*;
import org.strobe.gfx.transform.AbstractTransform;

public final class TransformSystem extends EntitySystem {

    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);

    private final EntityPool pool;

    public TransformSystem(EntityComponentSystem ecs) {
        super(ecs);

        pool = ecs.createPool(EntityFilter.requireAll(Transform.class));
    }

    @Override
    public void init() {}

    public void update(float dt) {
        for(Entity entity : pool){
            Transform transform = entity.get(TRANSFORM);
            Transform root = null;
            Entity parent = entity.getParent();
            while(parent != null){
                root = parent.get(TRANSFORM);
                if(root != null)break;
                parent = parent.getParent();
            }
            transform.relocate(root);
        }
        AbstractTransform.updateTransforms();
    }

    @Override
    public void dispose() {}
}
