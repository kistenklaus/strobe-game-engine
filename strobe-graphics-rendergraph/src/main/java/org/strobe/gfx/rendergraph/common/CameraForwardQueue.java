package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.lights.LightConstants;
import org.strobe.gfx.lights.ShadowUbo;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;
import org.strobe.gfx.transform.AbstractTransform;

import static org.lwjgl.opengl.GL11.*;

public class CameraForwardQueue extends RenderQueue {

    private final Resource<CameraManager> cameras = registerResource(CameraManager.class, "cameras");
    private final Resource<LightManager> lights = registerResource(LightManager.class, "lights");

    private ShadowUbo noShadowUbo;

    private Texture2D currentShadowMap = null;

    @Override
    protected void complete(Graphics gfx) {
        if (cameras.get() == null) throw new IllegalStateException();
        if (lights.get() == null) throw new IllegalStateException();

        noShadowUbo = new ShadowUbo(gfx);
        int[] lightIndices = new int[LightConstants.DIRECTIONAL_LIGHT_COUNT];
        for (int i = 0; i < lightIndices.length; i++) lightIndices[i] = -1;
        noShadowUbo.uniformDirLightIndices(gfx, lightIndices);
        noShadowUbo.uniformDirLightCastingCount(gfx, 0);
    }

    @Override
    public void renderDrawable(Graphics gfx, AbstractTransform transform, Renderable renderable, MaterialShader shader, Bindable[] bindables) {
        for (Bindable bindable : bindables) gfx.bind(bindable);
        gfx.bind(shader);
        if (currentShadowMap != null) {
            shader.uniformShadowMap(gfx, currentShadowMap);
        }
        shader.uniformModelMatrix(gfx, transform.getTransformationMatrix());
        renderable.render(gfx);
        gfx.unbind(shader);
        for (Bindable bindable : bindables) gfx.unbind(bindable);
    }

    @Override
    protected void renderQueue(Graphics gfx) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        gfx.bind(lights.get().ubo());
        for (AbstractCamera camera : cameras.get().cameras()) {
            gfx.bind(camera.getTarget());
            gfx.bind(camera.getCameraUbo());

            ShadowUbo shadowUbo = lights.get().getCameraShadowUbo(camera);
            Framebuffer shadowMap = lights.get().getCameraShadowMap(camera);

            if (shadowUbo != null && shadowMap != null) {
                gfx.bind(shadowUbo);
                currentShadowMap = shadowMap.getAttachmentTexture(Framebuffer.Attachment.SHADOW_DEPTH_ATTACHMENT);
            } else {
                gfx.bind(noShadowUbo);
            }

            for (RenderQueue.Job job : queue)
                job.execute(gfx);


            if (shadowUbo != null && shadowMap != null) {
                gfx.unbind(shadowUbo);
                currentShadowMap = null;
            } else {
                gfx.unbind(noShadowUbo);
            }

            gfx.unbind(camera.getCameraUbo());
            gfx.unbind(camera.getTarget());
        }
        gfx.unbind(lights.get().ubo());
    }

    public Resource<CameraManager> getCameraResource() {
        return cameras;
    }

    public Resource<LightManager> getLightResource() {
        return lights;
    }
}
