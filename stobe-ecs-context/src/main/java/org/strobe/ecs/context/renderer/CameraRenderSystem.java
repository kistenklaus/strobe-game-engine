package org.strobe.ecs.context.renderer;

import org.strobe.ecs.*;

public class CameraRenderSystem extends EntitySystem {

    private static final ComponentMapper<CameraRenderer> CAMERA_RENDERER = ComponentMapper.getFor(CameraRenderer.class);
    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);
    private static final ComponentMapper<CameraLense> CAMERA_LENSE = ComponentMapper.getFor(CameraLense.class);

    private final EntityPool pool;
    private final EntityRenderer renderer;

    public CameraRenderSystem(EntityComponentSystem ecs, EntityRenderer renderer) {
        super(ecs);
        this.renderer = renderer;
        pool = ecs.createPool(EntityFilter.requireAll(CameraRenderer.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for(Entity entity : pool){
            CameraRenderer cameraRenderer = entity.get(CAMERA_RENDERER);
            cameraRenderer.setTransform(entity.get(TRANSFORM));
            cameraRenderer.setLense(entity.get(CAMERA_LENSE));
            cameraRenderer.update();
            if(cameraRenderer.getAbstractCamera() != null)renderer.submitCamera(cameraRenderer.getAbstractCamera());
            cameraRenderer.enqueueCameraOps(renderer);
        }
    }

    @Override
    public void dispose() {

    }
}
