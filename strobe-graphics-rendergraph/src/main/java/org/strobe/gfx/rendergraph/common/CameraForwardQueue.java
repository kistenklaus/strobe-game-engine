package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;
import org.strobe.gfx.transform.AbstractTransform;

public class CameraForwardQueue extends RenderQueue {

    private final Resource<CameraManager> cameras;

    public CameraForwardQueue(){
        cameras = registerResource(CameraManager.class, "cameras");
    }


    @Override
    protected void complete(Graphics gfx) {

    }

    @Override
    public void renderDrawable(Graphics gfx, AbstractTransform transform, Renderable renderable, MaterialShader shader, Bindable[] bindables) {
        for(Bindable bindable : bindables)gfx.bind(bindable);
        gfx.bind(shader);
        shader.uniformModelMatrix(gfx, transform.getTransformationMatrix());
        renderable.render(gfx);
        gfx.unbind(shader);
        for(Bindable bindable : bindables)gfx.unbind(bindable);
    }

    @Override
    protected void renderQueue(Graphics gfx) {
        for(AbstractCamera camera : cameras.get().cameras()){
            gfx.bind(camera.getTarget());
            gfx.bind(camera.getCameraUbo());
            for(RenderQueue.Job job : queue)job.execute(gfx);
            gfx.unbind(camera.getCameraUbo());
            gfx.unbind(camera.getTarget());
        }
    }

    public Resource<CameraManager> getCameraResource(){
        return cameras;
    }
}
