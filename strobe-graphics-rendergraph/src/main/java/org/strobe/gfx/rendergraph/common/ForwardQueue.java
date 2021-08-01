package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.lights.LightManager;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;
import org.strobe.gfx.transform.AbstractTransform;

import static org.lwjgl.opengl.GL11.*;

public class ForwardQueue extends RenderQueue {

    private static final String TARGET_RESOURCE = "target";
    private static final String CAMERA_RESOURCE = "camera";
    private static final String LIGHTS_RESOURCE = "lights";
    private static final String SHADOW_MAP_RESOURCE = "shadowMap";

    private final Resource<Framebuffer> target;
    private final Resource<Ubo> camera;
    private final Resource<LightManager> lights;
    private final Resource<Framebuffer> shadowMap;

    public ForwardQueue(){
        target = registerResource(Framebuffer.class, TARGET_RESOURCE);
        camera = registerResource(Ubo.class, CAMERA_RESOURCE);
        lights = registerResource(LightManager.class, LIGHTS_RESOURCE);
        shadowMap = registerResource(Framebuffer.class, SHADOW_MAP_RESOURCE);
    }

    @Override
    public void renderDrawable(Graphics gfx, AbstractTransform transform, Renderable renderable, MaterialShader shader, Bindable[] bindables) {
        for(Bindable bindable : bindables)gfx.bind(bindable);
        gfx.bind(shader);
        shader.uniformModelMatrix(gfx, transform.getTransformationMatrix());
        //shader.uniformShadowMap(gfx,?);
        renderable.render(gfx);
        gfx.unbind(shader);
        for(Bindable bindable : bindables)gfx.unbind(bindable);
    }


    @Override
    protected void complete(Graphics gfx) {
        addBindable(target.get());
        //addBindable(camera.get());
        //addBindable(lights.get().ubo());
    }

    @Override
    public void beforeRender(Graphics gfx) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
    }

    public Resource<Framebuffer> getTargetResource(){
        return target;
    }

    public Resource<Ubo> getCameraResource(){
        return camera;
    }

    public Resource<LightManager> getLightResource(){
        return lights;
    }

    public Resource<Framebuffer> getShadowMapResource(){
        return shadowMap;
    }
}
