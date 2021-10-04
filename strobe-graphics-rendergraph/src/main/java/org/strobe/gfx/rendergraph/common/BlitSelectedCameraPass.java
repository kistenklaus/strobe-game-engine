package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

import static org.lwjgl.opengl.GL11.*;

public final class BlitSelectedCameraPass extends RenderPass {

    private final Resource<CameraManager> cameras = registerResource(CameraManager.class, "cameras");
    private final Resource<Framebuffer> target = registerResource(Framebuffer.class, "target");

    @Override
    protected void complete(Graphics gfx) {

    }

    @Override
    protected void reset(Graphics gfx) {
        if(cameras.get()==null)throw new IllegalStateException();
        if(target.get()==null)throw new IllegalStateException();
    }

    @Override
    protected void render(Graphics gfx) {
        AbstractCamera focusedCameras = cameras.get().getSelectedCamera();
        if(focusedCameras != null){
            focusedCameras.getTarget().copyTo(gfx,
                    target.get(), GL_COLOR_BUFFER_BIT,
                    focusedCameras.isEnabledLinearScaling() ? GL_LINEAR : GL_NEAREST);
            focusedCameras.getTarget().copyTo(gfx,
                    target.get(), GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);
        }else{
            glClearColor(1,1,0,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<CameraManager> getCameraResource() {
        return cameras;
    }

    public Resource<Framebuffer> getTargetResource() {
        return target;
    }
}
