package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.CameraUbo;

import java.util.ArrayList;

public final class CameraManager {

    private CameraUbo ubo;
    private final ArrayList<AbstractCamera> cameras = new ArrayList<>();
    private AbstractCamera selectedCamera = null;

    public CameraManager(Graphics gfx){
        ubo = new CameraUbo(gfx);
    }

    public void submitCamera(AbstractCamera camera){
        cameras.add(camera);
    }

    public void clearFrame(){
        cameras.clear();
        selectedCamera = null;
    }

    public Iterable<AbstractCamera> cameras(){
        return cameras;
    }

    public AbstractCamera getSelectedCamera(){
        return selectedCamera;
    }


    public void selectCamera(AbstractCamera camera) {
        selectedCamera = camera;
    }
}
