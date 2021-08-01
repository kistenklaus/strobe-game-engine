package org.strobe.gfx.rendergraph.common;

import org.joml.Matrix4f;
import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.lights.LightManager;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.io.ShaderLoader;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;
import org.strobe.gfx.transform.AbstractTransform;


public final class ShadowQueue extends RenderQueue {

    private static final String SHADOW_MAP_BUFFER_RESOURCE = "shadowMapBuffer";
    private static final String LIGHTS_RESOURCE = "lights";

    private static final String SHADOW_SHADER_DIRECTORY = "shadowMapping";

    private static final String TRANSFORM_UNIFORM = "modelMatrix";

    private final Resource<Framebuffer> shadowMapResource;
    private final Resource<LightManager> lightManagerResource;

    private final Shader shadowMapShader;
    private final Uniform<Matrix4f> modelMatrixUniform;

    public ShadowQueue(Graphics gfx) {
        shadowMapResource = registerResource(Framebuffer.class, SHADOW_MAP_BUFFER_RESOURCE);
        lightManagerResource = registerResource(LightManager.class, LIGHTS_RESOURCE);
        shadowMapShader= ShaderLoader.loadShader(gfx, SHADOW_SHADER_DIRECTORY);
        modelMatrixUniform = shadowMapShader.getUniform(Matrix4f.class, TRANSFORM_UNIFORM);
    }

    @Override
    protected void complete(Graphics gfx) {
        addBindable(shadowMapResource.get());
        addBindable(shadowMapShader);
    }

    @Override
    public void renderDrawable(Graphics gfx, AbstractTransform transform, Renderable mesh, MaterialShader shader, Bindable[] bindables) {

    }

    @Override
    protected void renderQueue(Graphics gfx) {
        for(RenderQueue.Job job : queue)job.execute(gfx);
    }

    public Resource<Framebuffer> getShadowMapResource() {
        return shadowMapResource;
    }

    public Resource<LightManager> getLightResource() {
        return lightManagerResource;
    }

}
