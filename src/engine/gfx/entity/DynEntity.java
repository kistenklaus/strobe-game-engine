package engine.gfx.entity;

import org.joml.Vector3f;

import engine.gfx.models.TexturedModel;

public class DynEntity {
	private TexturedModel tex_model;
	private Vector3f pos;
	private float rotX, rotY, rotZ;
	private float scale;
	public DynEntity(TexturedModel tex_model, Vector3f pos, float rotX, float rotY, float rotZ, float scale) {
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
		rotX+=d;
	}
	public void rotateY(float d) {
		rotY+=d;
	}
	public void rotateZ(float d) { 
		rotZ+=d;
	}
	public TexturedModel getTex_model() {
		return tex_model;
	}
	public void setTex_model(TexturedModel tex_model) {
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
