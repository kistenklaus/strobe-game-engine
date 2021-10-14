package org.strobe.ecs.context.renderer.camera;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.transform.Transform;

public class CameraSubmissionSystem extends EntitySystem {

    private static final ComponentMapper<CameraRenderer> CAMERA_RENDERER = ComponentMapper.getFor(CameraRenderer.class);
    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);
    private static final ComponentMapper<CameraLense> CAMERA_LENSE = ComponentMapper.getFor(CameraLense.class);
    private static final ComponentMapper<FocusCamera> FOCUS_CAMERA = ComponentMapper.getFor(FocusCamera.class);

    private final EntityPool cameraRenderers;
    private final EntityPool focusedCameras;
    private final EntityRenderer renderer;

    public CameraSubmissionSystem(EntityComponentSystem ecs, EntityRenderer renderer) {
        super(ecs);
        this.renderer = renderer;
        cameraRenderers = ecs.createPool(EntityFilter.requireAll(CameraRenderer.class, CameraLense.class));
        focusedCameras = ecs.createPool(EntityFilter.requireAll(FocusCamera.class, CameraRenderer.class, CameraLense.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for (Entity entity : cameraRenderers) {
            CameraRenderer cameraRenderer = entity.get(CAMERA_RENDERER);
            cameraRenderer.setTransform(entity.get(TRANSFORM));
            cameraRenderer.setLense(entity.get(CAMERA_LENSE));
            cameraRenderer.update();
            if (cameraRenderer.getAbstractCamera() != null){
                renderer.submitCamera(cameraRenderer.getAbstractCamera());
                if(cameraRenderer.isRenderingShadows()){
                    renderer.submitShadowCamera(cameraRenderer.getAbstractCamera());
                }
            }
            cameraRenderer.enqueueCameraOps(renderer);
        }
        if(!focusedCameras.isEmpty()){
            if(focusedCameras.getSize() != 1)
                throw new IllegalStateException("only one camera can should ever have a FocusCamera component");
            renderer.selectCamera(focusedCameras.getAny().get(CAMERA_RENDERER).getAbstractCamera());
        }
    }

    @Override
    public void dispose() {

    }
}
