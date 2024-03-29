package org.strobe.ecs.context.renderer.mesh;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.materials.Material;
import org.strobe.ecs.context.renderer.transform.Transform;

public final class MeshRendererSystem extends EntitySystem {

    private static final ComponentMapper<MeshRenderer> MESH_RENDERER = ComponentMapper.getFor(MeshRenderer.class);
    private static final ComponentMapper<Mesh> MESH = ComponentMapper.getFor(Mesh.class);
    private static final ComponentMapper<Material> MATERIAL = ComponentMapper.getFor(Material.class);
    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);

    private final EntityPool pool;

    private final EntityRenderer renderer;

    public MeshRendererSystem(EntityComponentSystem ecs, EntityRenderer renderer){
        super(ecs);
        this.renderer = renderer;
        pool = ecs.createPool(EntityFilter.requireAll(MeshRenderer.class));
    }

    @Override
    public void init() {}

    @Override
    public void update(float dt) {
        for(Entity entity : pool){
            MeshRenderer meshRenderer = entity.get(MESH_RENDERER);
            meshRenderer.update(renderer, entity.get(TRANSFORM), entity.get(MESH), entity.get(MATERIAL));
            renderer.draw(meshRenderer.graphDrawable);
        }
    }

    @Override
    public void dispose() {

    }
}
