package org.strobe.ecs.context.renderer;

import org.strobe.ecs.*;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.*;
import org.strobe.gfx.rendergraph.core.RenderGraphRenderer;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;

import java.util.Collection;
import java.util.LinkedList;
import java.util.function.BiConsumer;

import static org.lwjgl.opengl.GL11.*;

public final class EntityRenderer extends RenderGraphRenderer {

    private static final String GLOBAL_CAMERA_RESOURCE = "globalCameraResource";
    private static final String GLOBAL_LIGHTS_RESOURCE = "globalLightsResource";
    private static final String GLOBAL_BACK_BUFFER_RESOURCE = "globalBackBufferResource";
    private static final String GLOBAL_TARGET_BUFFER_RESOURCE = "globalTargetBufferResource";
    private static final String GLOBAL_POST_PROCESSING_BUFFER_RESOURCE = "globalPostProcessingBufferResource";
    private static final String GLOBAL_SHADOW_MAP_RESOURCE = "globalShadowMapResource";


    private enum RendererState {
        DISABLED_RENDERER,
        RENDERER_3D,
    }

    private RendererState rendererState = RendererState.DISABLED_RENDERER;

    private final CameraManager cameraManager;

    private final ClearCamerasPass clearCamerasPass;
    private final CameraUpdatePass cameraUpdatePass;
    private final CameraForwardQueue forwardQueue;
    private final CameraPostProcessingPass postProcessingPass;

    private final Resource<CameraManager> globalCameraResource;


    private final LinkedList<BiConsumer<Graphics, EntityRenderer>> renderOps = new LinkedList<>();

    public EntityRenderer(Graphics gfx, EntityComponentSystem ecs) {
        ecs.addEntitySystem(new MeshRendererSystem(ecs, this));
        ecs.addEntitySystem(new MaterialSystem(ecs, this));
        ecs.addEntitySystem(new TransformSystem(ecs));
        ecs.addEntitySystem(new CameraRenderSystem(ecs, this));

        cameraManager = new CameraManager(gfx);

        clearCamerasPass = new ClearCamerasPass(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cameraUpdatePass = new CameraUpdatePass();
        forwardQueue = new CameraForwardQueue();
        postProcessingPass = new CameraPostProcessingPass(gfx);

        addPass(clearCamerasPass);
        addPass(cameraUpdatePass);
        addPass(forwardQueue);
        addPass(postProcessingPass);

        globalCameraResource = registerResource(CameraManager.class, GLOBAL_CAMERA_RESOURCE, cameraManager);

        addLinkage(globalCameraResource, clearCamerasPass.getCameraResource());
        addLinkage(clearCamerasPass.getCameraResource(), cameraUpdatePass.getCameraResource());
        addLinkage(cameraUpdatePass.getCameraResource(), forwardQueue.getCameraResource());
        addLinkage(forwardQueue.getCameraResource(), postProcessingPass.getCameraResource());
    }

    public void submitCamera(AbstractCamera camera) {
        cameraManager.submitCamera(camera);
    }

    @Override
    public void beforeRender(Graphics gfx) {
        while(!renderOps.isEmpty())renderOps.pop().accept(gfx, this);
    }

    @Override
    public void afterRender(Graphics gfx) {
        AbstractCamera mainCamera = cameraManager.getSelectedCamera();
        if (mainCamera != null) {
            mainCamera.getTarget().copyTo(gfx, Framebuffer.getBackBuffer(gfx), GL_COLOR_BUFFER_BIT,
                    mainCamera.isEnabledLinearScaling()?GL_LINEAR:GL_NEAREST);
        } else {
            //TODO better No Camera Render
            glClearColor(1,0,1,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        cameraManager.clearFrame();
    }
    public RenderQueue getForwardQueue() {
        return forwardQueue;
    }

    public void enqueueRenderOps(Collection<BiConsumer<Graphics, EntityRenderer>> renderOp){
        renderOps.addAll(renderOp);
    }

    public void enqueueRenderOp(BiConsumer<Graphics, EntityRenderer> renderOp){
        renderOps.add(renderOp);
    }

}
