package org.strobe.gfx.rendergraph.common;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

import static org.lwjgl.opengl.GL11.glClear;
import static org.lwjgl.opengl.GL11.glClearColor;

public class ClearCamerasPass extends RenderPass {

    private final Resource<CameraManager> cameras;

    private final int mask;

    public ClearCamerasPass(int mask){
        this.mask = mask;
        cameras = registerResource(CameraManager.class, "cameras");
    }

    @Override
    protected void complete(Graphics gfx) {
        if(cameras.get() == null)throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        for(AbstractCamera camera : cameras.get().cameras()){
            Vector3f clearColor = camera.getBackgroundColor();
            glClearColor(clearColor.x, clearColor.y,clearColor.z, 1f);
            gfx.bind(camera.getTarget());
            glClear(mask);
            gfx.unbind(camera.getTarget());
            gfx.bind(camera.getBackTarget());
            glClear(mask);
            gfx.unbind(camera.getBackTarget());
        }
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<CameraManager> getCameraResource(){
        return cameras;
    }
}
