package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.materials.ShaderMaterial;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.materials.shaders.ShadowMapShader;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;
import org.strobe.gfx.transform.AbstractTransform;

import static org.lwjgl.opengl.GL11.*;

public final class ShadowQueue extends RenderQueue {

    private final Resource<LightManager> lights = registerResource(LightManager.class, "lights");

    private final ShadowMapShader shadowMapShader;

    public ShadowQueue(Graphics gfx) {
        shadowMapShader = new ShadowMapShader(gfx);
    }

    @Override
    protected void complete(Graphics gfx) {
        if (lights.get() == null) throw new IllegalStateException();
    }

    @Override
    public void beforeRender(Graphics gfx) {

    }

    @Override
    public void renderDrawable(Graphics gfx, AbstractTransform transform,
                               Renderable renderable, MaterialShader shader,
                               Bindable[] bindables) {
        shadowMapShader.uniformModelMatrix(gfx, transform.getTransformationMatrix());
        renderable.render(gfx);
    }

    @Override
    protected void renderQueue(Graphics gfx) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        gfx.bind(shadowMapShader);
        for (int i = 0; i < lights.get().getShadowCameraCount(); i++) {
            Ubo shadowUbo = lights.get().getShadowUbo(i);
            Framebuffer shadowMap = lights.get().getShadowMap(i);
            gfx.bind(shadowUbo);
            gfx.bind(shadowMap);
            glClearColor(0.1f,0.1f,0.1f,1);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            for(int j=0;j<lights.get().getDirCasterCount();j++){
                shadowMapShader.uniformLightIndex(gfx, j);
                for (RenderQueue.Job job : queue) job.execute(gfx);
            }

            gfx.unbind(shadowMap);
            gfx.unbind(shadowUbo);
        }
        gfx.unbind(shadowMapShader);
    }

    @Override
    public void afterRender(Graphics gfx) {

    }

    public Resource<LightManager> getLightResource() {
        return lights;
    }
}
