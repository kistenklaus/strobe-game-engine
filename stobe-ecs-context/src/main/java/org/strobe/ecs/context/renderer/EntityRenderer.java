package org.strobe.ecs.context.renderer;

import org.strobe.ecs.*;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.lights.AbstractLight;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderGraphRenderer;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;

import java.util.Collection;
import java.util.LinkedList;
import java.util.function.BiConsumer;

public abstract class EntityRenderer extends RenderGraphRenderer {

    protected static final String GLOBAL_CAMERA_RESOURCE = "globalCameraResource";
    protected static final String GLOBAL_LIGHTS_RESOURCE = "globalLightsResource";
    protected static final String GLOBAL_BACK_BUFFER_RESOURCE = "globalBackBufferResource";
    protected static final String GLOBAL_TARGET_BUFFER_RESOURCE = "globalTargetBufferResource";
    protected static final String GLOBAL_POST_PROCESSING_BUFFER_RESOURCE = "globalPostProcessingBufferResource";
    protected static final String GLOBAL_SHADOW_MAP_RESOURCE = "globalShadowMapResource";

    protected final LinkedList<BiConsumer<Graphics, EntityRenderer>> renderOps = new LinkedList<>();

    protected final LightManager lightManager;
    protected final CameraManager cameraManager;

    protected final Resource<CameraManager> globalCameraResource;
    protected final Resource<Framebuffer> globalBackBuffer;
    protected final Resource<LightManager> globalLightResource;

    public EntityRenderer(Graphics gfx, EntityComponentSystem ecs, Framebuffer target) {
        this.lightManager = new LightManager(gfx);
        this.cameraManager = new CameraManager(gfx);

        globalCameraResource = registerResource(CameraManager.class, GLOBAL_CAMERA_RESOURCE, cameraManager);
        globalBackBuffer = registerResource(Framebuffer.class, GLOBAL_BACK_BUFFER_RESOURCE, target);
        globalLightResource = registerResource(LightManager.class, GLOBAL_LIGHTS_RESOURCE, lightManager);
    }

    @Override
    public void beforeRender(Graphics gfx) {
        while(!renderOps.isEmpty())renderOps.pop().accept(gfx, this);
    }

    public void enqueueRenderOps(Collection<BiConsumer<Graphics, EntityRenderer>> renderOp) {
        renderOps.addAll(renderOp);
    }

    public void enqueueRenderOp(BiConsumer<Graphics, EntityRenderer> renderOp) {
        renderOps.add(renderOp);
    }


    public abstract RenderQueue getForwardQueue();

    public abstract RenderQueue getShadowQueue();

    public void submitCamera(AbstractCamera camera){
        cameraManager.submitCamera(camera);
    }

    public void submitShadowCamera(AbstractCamera camera){
        lightManager.submitShadowCamera(camera);
    }

    public void selectCamera(AbstractCamera camera){
        cameraManager.selectCamera(camera);
    }

    public void submitLight(AbstractLight light){
        lightManager.submitLight(light);
    }

}

