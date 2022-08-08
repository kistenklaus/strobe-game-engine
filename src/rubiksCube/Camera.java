package rubiksCube;

import org.joml.Vector3f;
import org.lwjgl.glfw.GLFW;

import engine.input.Input;

public class Camera {
	private static final float CAM_SPEED = 0.1f;
	private Vector3f pos;
	private float pitch, yaw, roll;
	public Camera(Vector3f pos) {
		this.pos = pos;
		this.pitch = 0;
		this.yaw = 0;
		this.roll = 0;
	}
	public void inputHandling(Input input) {
		if(input.isKeyDown(GLFW.GLFW_KEY_A)) {
			incressPos(-CAM_SPEED,0,0);
		}if(input.isKeyDown(GLFW.GLFW_KEY_D)) {
			incressPos(CAM_SPEED,0,0);
		}if(input.isKeyDown(GLFW.GLFW_KEY_W)) {
			incressPos(0,CAM_SPEED,0);
		}if(input.isKeyDown(GLFW.GLFW_KEY_S)) {
			incressPos(0,-CAM_SPEED,0);
		}
		incressPos(0, 0, (float) -input.getScrollY());
		input.updateScroll();
	}
	public void incressPos(Vector3f delta) {
		incressPos(delta.x, delta.y,delta.z);
	}
	public void incressPos(float dx, float dy, float dz) {
		this.pos.x+=dx;
		this.pos.y+=dy;
		this.pos.z+=dz;
	}
	public Vector3f getPos() {
		return pos;
	}
	public void setPos(Vector3f pos) {
		this.pos = pos;
	}
	public float getPitch() {
		return pitch;
	}
	public void setPitch(float pitch) {
		this.pitch = pitch;
	}
	public float getYaw() {
		return yaw;
	}
	public void setYay(float yaw) {
		this.yaw = yaw;
	}
	public float getRoll() {
		return roll;
	}
	public void setRoll(float roll) {
		this.roll = roll;
	}
	
}
