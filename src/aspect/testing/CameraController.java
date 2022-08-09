package aspect.testing;

import org.aspect.graphics.data.Camera;
import org.aspect.window.event.WindowEvent;
import org.aspect.window.listener.WindowKeyHandler;
import org.aspect.window.listener.WindowMouseHandler;
import org.joml.Vector3f;

public class CameraController {

    private final float PEDESTAL_SPEED = 50;
    private final float TRUCK_SPEED = 60;
    private final float DOLLY_SPEED = 70;

    private Camera camera;
    private WindowMouseHandler mouse;
    private WindowKeyHandler keyboard;

    public CameraController(Camera camera, WindowMouseHandler mouse,
                            WindowKeyHandler keyboard) {
        this.camera = camera;
        this.mouse = mouse;
        this.keyboard = keyboard;
    }

    public void update(double dt) {
        if (mouse.isButtonDown(WindowEvent.MOUSE_BUTTON_1)) {
            camera.pitch(-mouse.getDeltaMouseX() * camera.getFov() * 0.075f);
            camera.roll(-mouse.getDeltaMouseY() * camera.getFov() * 0.075f * camera.getAspectRatio());
        }

        if (keyboard.isKeyDown(WindowEvent.KEY_A)) {
            camera.truck((float) -dt * TRUCK_SPEED);
        }
        if (keyboard.isKeyDown(WindowEvent.KEY_D)) {
            camera.truck((float) dt * TRUCK_SPEED);
        }

        if (keyboard.isKeyDown(WindowEvent.KEY_W)) {
            Vector3f forward = new Vector3f(camera.getForward().x, 0, camera.getForward().z);
            forward.normalize().mul((float) dt * DOLLY_SPEED);
            camera.move(forward.x, 0, forward.z);
        }
        if (keyboard.isKeyDown(WindowEvent.KEY_S)) {
            Vector3f backward = new Vector3f(-camera.getForward().x, 0, -camera.getForward().z);
            backward.normalize().mul((float) dt * DOLLY_SPEED);
            camera.move(backward.x, 0, backward.z);
        }
        if (keyboard.isKeyDown(WindowEvent.KEY_SPACE)) {
            camera.move(new Vector3f(0, (float) dt * PEDESTAL_SPEED, 0));
        }
        if (keyboard.isKeyDown(WindowEvent.KEY_LEFT_SHIFT)) {
            camera.move(new Vector3f(0, (float) -dt * PEDESTAL_SPEED, 0));
        }
    }
}
