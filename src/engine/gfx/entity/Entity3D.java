package engine.gfx.entity;

import org.joml.Vector3f;

import engine.gfx.models.Model3D;

public class Entity3D {
	protected Model3D tex_model;
	protected Vector3f pos;
	protected float rotX, rotY, rotZ;
	protected float scale;
	public Entity3D(Model3D tex_model, Vector3f pos, float rotX, float rotY, float rotZ, float scale) {
		super();
		this.tex_model = tex_model;
		this.pos = pos;
		this.rotX = rotX;
		this.rotY = rotY;
		this.rotZ = rotZ;
		this.scale = scale;
	}
	public void incressPos(Vector3f delta) {
		incressPos(delta.x, delta.y,delta.z);
	}
	public void incressPos(float dx, float dy, float dz) {
		this.pos.x+=dx;
		this.pos.y+=dy;
		this.pos.z+=dz;
	}
	public void rotate(float dx, float dy, float dz) {
		rotateX(dx);
		rotateY(dy);
		rotateZ(dz);
	}
	public void rotateX(float d) {
		rotX+= Math.toRadians(d);
	}
	public void rotateY(float d) {
		rotY+= Math.toRadians(d);
	}
	public void rotateZ(float d) { 
		rotZ+= Math.toRadians(d);
	}
	public Model3D getTex_model() {
		return tex_model;
	}
	public void setTex_model(Model3D tex_model) {
		this.tex_model = tex_model;
	}
	public Vector3f getPos() {
		return pos;
	}
	public void setPos(Vector3f pos) {
		this.pos = pos;
	}
	public float getRotX() {
		return rotX;
	}
	public void setRotX(float rotX) {
		this.rotX = rotX;
	}
	public float getRotY() {
		return rotY;
	}
	public void setRotY(float rotY) {
		this.rotY = rotY;
	}
	public float getRotZ() {
		return rotZ;
	}
	public void setRotZ(float rotZ) {
		this.rotZ = rotZ;
	}
	public float getScale() {
		return scale;
	}
	public void setScale(float scale) {
		this.scale = scale;
	}
	
}
