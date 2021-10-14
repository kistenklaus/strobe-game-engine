package org.strobe.engine.release;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.context.EntityContext;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.camera.CameraSubmissionSystem;
import org.strobe.ecs.context.renderer.light.LightSubmissionSystem;
import org.strobe.ecs.context.renderer.materials.MaterialSystem;
import org.strobe.ecs.context.renderer.mesh.MeshRendererSystem;
import org.strobe.ecs.context.renderer.transform.TransformSystem;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.rendergraph.common.*;
import org.strobe.gfx.rendergraph.common.debugpasses.CameraDebugPass;
import org.strobe.gfx.rendergraph.common.debugpasses.LightDebugPass;
import org.strobe.gfx.rendergraph.core.RenderQueue;

import static org.lwjgl.opengl.GL11.GL_COLOR_BUFFER_BIT;
import static org.lwjgl.opengl.GL11.GL_DEPTH_BUFFER_BIT;

public final class ReleaseRenderer extends EntityRenderer {

    private final ClearCamerasPass clearCamerasPass;
    private final CameraUpdatePass cameraUpdatePass;
    private final CameraForwardQueue forwardQueue;
    private final CameraPostProcessingPass postProcessingPass;
    private final BlitSelectedCameraPass blitCameraPass;
    private final LightUpdatePass lightUpdatePass;
    private final CameraDebugPass cameraDebugPass;
    private final LightDebugPass lightDebugPass;
    private final ShadowQueue shadowQueue;

    public ReleaseRenderer(Graphics gfx, EntityContext context) {
        super(gfx, context.getEcs(), context.getTarget());

        EntityComponentSystem ecs = context.getEcs();

        ecs.addEntitySystem(new MeshRendererSystem(ecs, this));
        ecs.addEntitySystem(new MaterialSystem(ecs, this));
        ecs.addEntitySystem(new TransformSystem(ecs));
        ecs.addEntitySystem(new CameraSubmissionSystem(ecs, this));
        ecs.addEntitySystem(new LightSubmissionSystem(ecs, this));

        clearCamerasPass = new ClearCamerasPass(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cameraUpdatePass = new CameraUpdatePass();
        forwardQueue = new CameraForwardQueue();
        postProcessingPass = new CameraPostProcessingPass(gfx);
        blitCameraPass = new BlitSelectedCameraPass();
        lightUpdatePass = new LightUpdatePass();
        cameraDebugPass = new CameraDebugPass(gfx);
        lightDebugPass = new LightDebugPass(gfx);
        shadowQueue = new ShadowQueue(gfx);

        addPass(clearCamerasPass);
        addPass(cameraUpdatePass);
        addPass(forwardQueue);
        addPass(postProcessingPass);
        addPass(blitCameraPass);
        addPass(lightUpdatePass);
        addPass(cameraDebugPass);
        addPass(lightDebugPass);
        addPass(shadowQueue);

        addLinkage(globalLightResource, lightUpdatePass.getLightResource());
        addLinkage(lightUpdatePass.getLightResource(), shadowQueue.getLightResource());
        addLinkage(shadowQueue.getLightResource(), forwardQueue.getLightResource());
        addLinkage(forwardQueue.getLightResource(), lightDebugPass.getLightResource());

        addLinkage(globalCameraResource, clearCamerasPass.getCameraResource());
        addLinkage(clearCamerasPass.getCameraResource(), cameraUpdatePass.getCameraResource());
        addLinkage(cameraUpdatePass.getCameraResource(), forwardQueue.getCameraResource());
        addLinkage(forwardQueue.getCameraResource(), postProcessingPass.getCameraResource());
        addLinkage(postProcessingPass.getCameraResource(), blitCameraPass.getCameraResource());
        addLinkage(blitCameraPass.getCameraResource(), cameraDebugPass.getCameraResource());
        addLinkage(cameraDebugPass.getCameraResource(), lightDebugPass.getCameraResource());

        addLinkage(globalBackBuffer, blitCameraPass.getTargetResource());
        addLinkage(blitCameraPass.getTargetResource(), cameraDebugPass.getTargetResource());
        addLinkage(cameraDebugPass.getTargetResource(), lightDebugPass.getTargetResource());
    }

    @Override
    public void afterRender(Graphics gfx) {
        cameraManager.clearFrame();
        lightManager.clearFrame();
    }

    public RenderQueue getForwardQueue() {
        return forwardQueue;
    }

    public RenderQueue getShadowQueue(){
        return shadowQueue;
    }

}
