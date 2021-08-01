package org.strobe.ecs;

public abstract class ComponentSystem<C extends Component> extends EntitySystem{

    protected final EntityPool componentPool;
    protected final ComponentMapper<C> componentMapper;

    public ComponentSystem(EntityComponentSystem ecs, Class<C> componentClass) {
        super(ecs);
        componentPool = ecs.createPool(EntityFilter.requireAll(componentClass));
        componentPool.addEntityAddedObserver(this::entityAdded);
        componentPool.addEntityRemovedObserver(this::entityRemoved);
        for(Entity entity : componentPool){
            entityAdded(entity);
        }
        componentMapper = ComponentMapper.getFor(componentClass);
    }

    public void init(){}

    protected abstract void entityAdded(Entity entity);
    protected abstract void entityRemoved(Entity entity);

    public void update(float dt){}

    @Override
    public final void dispose() {
        componentPool.removeEntityAddedObserver(this::entityAdded);
        componentPool.removeEntityRemovedObserver(this::entityRemoved);
    }
}
