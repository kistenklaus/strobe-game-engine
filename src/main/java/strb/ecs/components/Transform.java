package strb.ecs.components;

import org.joml.Vector3f;
import strb.ecs.Component;

public class Transform implements Component{
	
	private Vector3f position;
	private Vector3f rotation;
	private Vector3f scale;
	private Vector3f rotationCenter;
	
	public Transform() {
		this.position = new Vector3f(0.0f, 0.0f, 0.0f);
		this.rotation = new Vector3f(0.0f, 0.0f, 0.0f);
		this.scale = new Vector3f(1.0f, 1.0f, 1.0f);
		this.rotationCenter = new Vector3f(0.0f, 0.0f, 0.0f);
	}
	
	public Vector3f getPosition() {
		return position;
	}
	
	public void setPosition(Vector3f position) {
		this.position = position;
	}
	
	public Vector3f getRotation() {
		return rotation;
	}
	
	public void setRotation(Vector3f rotation) {
		this.rotation = rotation;
	}
	
	public Vector3f getScale() {
		return scale;
	}
	
	public void setScale(Vector3f scale) {
		this.scale = scale;
	}
	
	public Vector3f getRotationCenter() {
		return rotationCenter;
	}
	
	public void setRotationCenter(Vector3f rotationCenter) {
		this.rotationCenter = rotationCenter;
	}
	
	
	
	
}
