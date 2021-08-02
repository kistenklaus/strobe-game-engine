package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.CameraFilter;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.core.FullScreenPass;
import org.strobe.gfx.rendergraph.core.Resource;

import static org.lwjgl.opengl.GL11.*;

import org.strobe.gfx.camera.filters.FXAAFilter;

public class CameraPostProcessingPass extends FullScreenPass {

    private final Resource<CameraManager> cameras;

    private final FXAAFilter fxaaFilter;

    public CameraPostProcessingPass(Graphics gfx){
        super(gfx);
        cameras = registerResource(CameraManager.class, "cameras");

        fxaaFilter = new FXAAFilter(gfx);
    }

    @Override
    protected void complete(Graphics gfx) {
        if(cameras.get()==null)throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        for(AbstractCamera camera : cameras.get().cameras()){
            for(CameraFilter filter : camera.filters()){
                gfx.bind(camera.getBackTarget());
                filter.beforeFSR(gfx, camera.getTarget());
                screen.render(gfx);
                filter.afterFSR(gfx);
                gfx.unbind(camera.getBackTarget());
                camera.swapBuffers();
            }
        }
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<CameraManager> getCameraResource(){
        return cameras;
    }
}
