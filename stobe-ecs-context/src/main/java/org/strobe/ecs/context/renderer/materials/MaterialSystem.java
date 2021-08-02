package org.strobe.ecs.context.renderer.materials;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.EntityRenderer;

public final class MaterialSystem extends EntitySystem {

    private static final ComponentMapper<Material> MATERIAL = ComponentMapper.getFor(Material.class);

    private final EntityRenderer renderer;

    private final EntityPool pool;

    public MaterialSystem(EntityComponentSystem ecs, EntityRenderer renderer) {
        super(ecs);
        this.renderer = renderer;
        this.pool = ecs.createPool(EntityFilter.requireAll(Material.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for(Entity entity : pool){
            entity.get(MATERIAL).enqueueMaterialOps(renderer);
        }
    }

    @Override
    public void dispose() {

    }
}
