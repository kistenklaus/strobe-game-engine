package engine.gfx.entity;

import org.joml.Vector2f;

import engine.gfx.models.Model2D;
import engine.gfx.textures.Texture;

public class Entity2D {
	private Model2D model;
	private Vector2f pos;
	private int level;
	private float rotation;
	private float scale;
	
	public Entity2D(Model2D model, Vector2f pos, int level, float rotation, float scale) {
		this.model = model;
		this.pos = pos;
		this.level = level;
		this.rotation = rotation;
		this.scale = scale;
	}
	public void incressPos(float dx, float dy) {
		this.pos.x += dx;
		this.pos.y += dy;
	}
	public void rotate(float angle) {
		this.rotation = (float) Math.toRadians(angle);
	}
	public Model2D getModel() {
		return model;
	}
	public void setModel(Model2D model) {
		this.model = model;
	}
	public void setTexture(Texture texture) {
		this.model.setTexture(texture);
	}
	public Vector2f getPos() {
		return pos;
	}
	public void setPos(Vector2f pos) {
		this.pos = pos;
	}
	public int getLevel() {
		return level;
	}
	public void setLevel(int level) {
		this.level = level;
	}
	public float getRotation() {
		return rotation;
	}
	public void setRotation(float rotation) {
		this.rotation = rotation;
	}
	public float getScale() {
		return scale;
	}
	public void setScale(float scale) {
		this.scale = scale;
	}
}
