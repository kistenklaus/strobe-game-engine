package game;

import org.joml.Vector3f;
import org.lwjgl.glfw.GLFW;

import engine.input.Input;

public class Camera {
	private static final float CAM_SPEED = 10f;
	private Vector3f pos;
	private float pitch, yaw, roll;
	private double lastmX, lastmY;
	public Camera(Vector3f pos) {
		this.pos = pos;
		this.pitch = 0;
		this.yaw = 0;
		this.roll = 0;
	}
	public void inputHandling(Input input) {
		double mx = input.getMouseX();
		double my = input.getMouseY();
		double dmx = lastmX - mx;
		double dmy = lastmY - my;
		this.lastmX = mx;
		this.lastmY = my;
		if(input.isMouseButtonDown(GLFW.GLFW_MOUSE_BUTTON_2)) {
			rotatePitch((float)-dmy/GameLogic.HEIGHT*200);
			rotateYaw((float)-dmx/GameLogic.WIDTH*200);
		}
		double scrollY = -input.getScrollY();
		if(scrollY != 0) {
			float dx = (float) (Math.sin(Math.toRadians(-getYaw()))*scrollY);
			float dz = (float) (Math.cos(Math.toRadians(-getYaw()))*scrollY);
			float dy = (float) (Math.sin(Math.toRadians(getPitch()))*scrollY);
			incressPos(dx * CAM_SPEED , dy*CAM_SPEED, dz*CAM_SPEED);
		}
		
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
	public void rotatePitch(float dpitch) {
		this.pitch += dpitch;
	}
	public void rotateYaw(float dyaw) {
		this.yaw += dyaw;
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
