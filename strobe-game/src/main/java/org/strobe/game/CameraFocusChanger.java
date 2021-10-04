package org.strobe.game;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.camera.CameraLense;
import org.strobe.ecs.context.renderer.camera.CameraRenderer;
import org.strobe.ecs.context.renderer.camera.FocusCamera;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.window.WindowKey;
import org.strobe.window.WindowKeyboard;

public final class CameraFocusChanger extends EntitySystem {


    private final WindowKeyboard keyboard;

    private final EntityPool pool;

    public CameraFocusChanger(EntityComponentSystem ecs, WindowKeyboard keyboard) {
        super(ecs);
        this.keyboard = keyboard;

        pool = ecs.createPool(EntityFilter.requireAll(CameraLense.class, Transform.class, CameraRenderer.class, CameraIndex.class));
    }

    @Override
    public void init() {
    }

    @Override
    public void update(float dt) {
        if(keyboard.isKeyDown(WindowKey.KEY_1)){
            selectCameraWithIndex(1);
        }else if(keyboard.isKeyDown(WindowKey.KEY_2)){
            selectCameraWithIndex(2);
        }else if(keyboard.isKeyDown(WindowKey.KEY_3)){
            selectCameraWithIndex(3);
        }else if(keyboard.isKeyDown(WindowKey.KEY_4)){
            selectCameraWithIndex(4);
        }
    }

    private void selectCameraWithIndex(int index) {
        for(Entity entity : pool){
            if(entity.get(CameraIndex.class).getIndex() == index){
                //unselect current:
                for(Entity e : pool) {
                   if(e.has(FocusCamera.class)) {
                       e.removeComponent(FocusCamera.class);
                   }
                   entity.addComponent(new FocusCamera());
                }
                return;
            }
        }
        System.out.println("WARNING: Could not select Camera with index : " + index);
    }

    @Override
    public void dispose() {

    }
}
