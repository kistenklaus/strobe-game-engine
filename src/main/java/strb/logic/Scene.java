package strb.logic;

import strb.core.Engine;
import strb.ecs.*;

public abstract class Scene {
	
	protected Engine engine;
	private EntityComponentSystem ecs;
	
	public Scene() {
		ecs = new EntityComponentSystem();
	}
	
	public abstract void preload() throws InterruptedException;
	
	public abstract void start();
	
	public void updateECS(float dt) {
		ecs.update(dt);
	}
	
	public void update(float dt) { }
	
	public void addEntity(Entity entity) {
		ecs.addEntity(entity);
	}
	
	public void removeEntity(Entity entity) {
		ecs.addEntity(entity);
	}
	
	public void addSystem(EntitySystem system) {
		ecs.addSystem(system);
	}
	
	public void removeSystem(EntitySystem system) {
		ecs.removeSystem(system);
	}
	
	public Engine getEngine() {
		return engine;
	}
	
	public void setEngine(Engine engine) {
		this.engine = engine;
	}
	
}
