package org.strobe.ecs.context.renderer.camera;

import org.strobe.ecs.Component;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.LenseCamera;

import java.util.ArrayList;
import java.util.function.BiConsumer;

public final class CameraRenderer implements Component {

    private final ArrayList<BiConsumer<Graphics, EntityRenderer>> cameraOps = new ArrayList<>();

    private AbstractCamera camera = null;
    private CameraLense lense = null;
    private Transform transform = null;

    public CameraRenderer(int horResolution, int verResolution){
        cameraOps.add((gfx, renderer)->{
            camera = new LenseCamera(gfx, horResolution, verResolution, ()->lense, ()->transform);
        });
    }

    public void enableLinearScaling(){
        cameraOps.add((gfx,r)->camera.enableLinearScaling());
    }

    public void disableLinearScaling(){
        cameraOps.add((gfx,r)->camera.disableLinearScaling());
    }

    public void enableFXAA(){
        cameraOps.add((gfx,r)->camera.enableFXAA(gfx));
    }

    public void disableFXAA(){
        cameraOps.add((gfx,r)->camera.disableFXAA());
    }

    protected void enqueueCameraOps(EntityRenderer renderer){
        renderer.enqueueRenderOps(cameraOps);
        cameraOps.clear();
    }

    public void update(){
        if(camera!=null)camera.update();
    }

    protected AbstractCamera getAbstractCamera(){
        return camera;
    }

    protected void setTransform(Transform transform){
        this.transform = transform;
    }

    protected void setLense(CameraLense lense){
        this.lense = lense;
    }

}
