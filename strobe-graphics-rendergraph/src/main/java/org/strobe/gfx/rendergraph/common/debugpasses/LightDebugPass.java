package org.strobe.gfx.rendergraph.common.debugpasses;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

public final class LightDebugPass extends RenderPass {

    private final Resource<LightManager> lights = registerResource(LightManager.class, "lights");
    private final Resource<CameraManager> cameras = registerResource(CameraManager.class, "cameras");
    private final Resource<Framebuffer> target = registerResource(Framebuffer.class, "target");

    public LightDebugPass(Graphics gfx){

    }

    @Override
    protected void complete(Graphics gfx) {
        if(lights.get() == null)throw new IllegalStateException();
        if(cameras.get() == null)throw new IllegalStateException();
        if(target.get() == null)throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        gfx.bind(target.get());



        gfx.unbind(target.get());
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<LightManager> getLightResource() {
        return lights;
    }

    public Resource<CameraManager> getCameraResource() {
        return cameras;
    }

    public Resource<Framebuffer> getTargetResource() {
        return target;
    }
}
