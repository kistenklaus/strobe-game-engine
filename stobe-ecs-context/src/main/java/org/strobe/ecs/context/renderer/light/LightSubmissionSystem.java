package org.strobe.ecs.context.renderer.light;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.gfx.lights.AbstractLight;


public final class LightSubmissionSystem extends EntitySystem {

    private static final ComponentMapper<LightComponent> LIGHT_COMPONENT = ComponentMapper.getFor(LightComponent.class);
    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);

    private final EntityRenderer renderer;
    private final EntityPool pool;

    public LightSubmissionSystem(EntityComponentSystem ecs, EntityRenderer renderer) {
        super(ecs);
        this.renderer = renderer;

        pool = ecs.createPool(EntityFilter.requireAll(LightComponent.class, Transform.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for(Entity entity : pool){
            LightComponent lightComponent = entity.get(LIGHT_COMPONENT);
            lightComponent.enqueueLightOps(renderer);
            AbstractLight light = lightComponent.getAbstractLight();
            light.setPosition(entity.get(TRANSFORM).getPosition());
            renderer.submitLight(light);
        }
    }

    @Override
    public void dispose() {

    }
}
