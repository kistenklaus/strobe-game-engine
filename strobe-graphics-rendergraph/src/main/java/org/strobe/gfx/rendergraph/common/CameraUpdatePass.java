package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

public class CameraUpdatePass extends RenderPass {

    private final Resource<CameraManager> cameras;

    public CameraUpdatePass() {
        this.cameras = registerResource(CameraManager.class, "cameras");
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
            camera.getCameraUbo().uniformCamera(gfx, camera);
        }
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<CameraManager> getCameraResource(){
        return cameras;
    }
}
