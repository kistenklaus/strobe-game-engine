package strb.ecs;


public class EntitySystem {

	private int priority = 0;
	
	public void onAdd(EntityComponentSystem entityComponentSystem) { }

	public void onRemove(EntityComponentSystem entityComponentSystem) { }
	
	public void update(float deltaTime) { }
	
	public int getPriority() {
		return this.priority;
	}
}
