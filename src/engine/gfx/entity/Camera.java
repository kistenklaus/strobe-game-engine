package engine.gfx.entity;

import org.joml.Vector3f;

public class Camera {
	private Vector3f pos;
	private float pitch, yaw, roll;
	public Camera(Vector3f pos) {
		this.pos = pos;
		this.pitch = 0;
		this.yaw = 0;
		this.roll = 0;
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
