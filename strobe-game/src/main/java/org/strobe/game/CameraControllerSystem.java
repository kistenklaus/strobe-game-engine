package org.strobe.game;

import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.Transform;
import org.strobe.window.WindowButton;
import org.strobe.window.WindowKey;
import org.strobe.window.WindowKeyboard;
import org.strobe.window.WindowMouse;


public final class CameraControllerSystem extends EntitySystem {

    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);

    private static final float MOVEMENT_SPEED = 10;
    private static final float MOUSE_SENSITIVITY = 25;

    private final EntityPool controllerPool;
    private final WindowMouse mouse;
    private final WindowKeyboard keyboard;

    public CameraControllerSystem(EntityComponentSystem ecs, WindowMouse mouse, WindowKeyboard keyboard) {
        super(ecs);
        this.mouse = mouse;
        this.keyboard = keyboard;
        controllerPool = ecs.createPool(EntityFilter.requireAll(CameraController.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for(Entity entity : controllerPool){
            Transform transform = entity.get(TRANSFORM);

            Vector3f translation = new Vector3f();
            if(keyboard.isKeyDown(WindowKey.KEY_W)){
                translation.add(transform.getForward().mul(new Vector3f(1,0,1).normalize()));
            }
            if(keyboard.isKeyDown(WindowKey.KEY_S)){
                translation.add(transform.getBack().mul(new Vector3f(1,0,1).normalize()));
            }

            if(keyboard.isKeyDown(WindowKey.KEY_A)){
                translation.add(transform.getLeft().mul(new Vector3f(1,0,1).normalize()));
            }
            if(keyboard.isKeyDown(WindowKey.KEY_D)){
                translation.add(transform.getRight().mul(new Vector3f(1,0,1).normalize()));
            }
            if(keyboard.isKeyDown(WindowKey.KEY_SPACE)){
                translation.add(new Vector3f(0,1,0));
            }
            if(keyboard.isKeyDown(WindowKey.KEY_LEFT_SHIFT)){
                translation.add(new Vector3f(0,-1,0));
            }

            transform.translate(translation.mul(MOVEMENT_SPEED * dt));

            Quaternionf orientation = transform.getOrientation();

            if(mouse.isButtonDown(WindowButton.LEFT)){
                orientation.mul(new Quaternionf().fromAxisAngleDeg(1,0,0, (float) (mouse.getDY()* MOUSE_SENSITIVITY *dt)));
                orientation.premul(new Quaternionf().fromAxisAngleDeg(0,1,0, (float) (mouse.getDX() * MOUSE_SENSITIVITY * dt)));
            }
            transform.setOrientation(orientation);

        }
    }

    @Override
    public void dispose() {

    }
}
