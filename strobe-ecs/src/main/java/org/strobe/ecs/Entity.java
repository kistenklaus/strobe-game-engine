package org.strobe.ecs;

import org.strobe.utils.UnboundedArray;

import java.util.*;

public final class Entity {

    private final EntityComponentSystem ecs;
    private final UnboundedArray<Component> components = new UnboundedArray<>(8);

    private final Set<Integer> poolIndices = new HashSet<>();

    private int entityIndex = -1;
    private boolean alive = false;

    private final ArrayList<ComponentObserver> componentAddedObservers = new ArrayList<>();
    private final ArrayList<ComponentObserver> componentRemovedObserver = new ArrayList<>();

    private int parent = -1;
    private final HashSet<Integer> children = new HashSet<>();

    protected Entity(EntityComponentSystem ecs) {
        this.ecs = ecs;
    }

    /**
     *
     * @param componentMapper
     * @return
     */
    public boolean has(ComponentMapper componentMapper){
        return has(componentMapper.getComponentType());
    }
    /**
     * @param componentClass
     * @return
     */
    public boolean has(Class<? extends Component> componentClass) {
        return has(ComponentType.getFor(componentClass));
    }

    protected boolean has(ComponentType componentType) {
        return components.get(componentType.getIndex()) != null;
    }

    /**
     *
     * @param componentClass
     * @param <T>
     * @return
     */
    public <T extends Component> T get(Class<T> componentClass){
        return (T)get(ComponentType.getFor(componentClass));
    }

    /**
     *
     * @param componentMapper
     * @param <T>
     * @return
     */
    public <T extends Component> T get(ComponentMapper<T> componentMapper){
        return (T) get(componentMapper.getComponentType());
    }

    protected Component get(ComponentType componentType){
        return components.get(componentType.getIndex());
    }


    /**
     *
     * @param componentMapper
     * @param component
     */
    public void addComponent(ComponentMapper componentMapper, Component component){
        addComponent(component, componentMapper.getComponentType());
    }
    /**
     * @param component
     */
    public void addComponent(Component component) {
        addComponent(component, ComponentType.getFor(component.getClass()));
    }

    protected void addComponent(Component component, ComponentType componentType) {
        //boolean removed = removeComponent(componentType, false);
        ecs.enqueueEntityOperation(()->addComponentInternal(component, componentType));
        //update entity pools
        //we need to update all entity pools because we could have filters like not ComponentA.
        ecs.enqueueEntityComponentChange(this, componentType);
    }

    protected void addComponentInternal(Component component, ComponentType componentType) {
        boolean override = components.get(componentType.getIndex()) != null;
        components.set(componentType.getIndex(), component);
        if(override)ecs.enqueueEntityComponentEvent(this, componentType, false);
        ecs.enqueueEntityComponentEvent(this, componentType, true);
    }

    /**
     *
     * @param componentMapper
     * @return
     */
    public boolean removeComponent(ComponentMapper componentMapper){
        return removeComponent(componentMapper.getComponentType(), true);
    }

    /**
     * @param component
     * @return
     */
    public boolean removeComponent(Component component) {
        return removeComponent(component.getClass());
    }

    /**
     * @param componentClass
     * @return
     */
    public boolean removeComponent(Class<? extends Component> componentClass) {
        return removeComponent(ComponentType.getFor(componentClass), true);
    }

    protected boolean removeComponent(ComponentType componentType, boolean enqueuePoolUpdate) {
        ecs.enqueueEntityOperation(()->removeComponentInternal(componentType));
        //update entity pools
        if (enqueuePoolUpdate) {
            ecs.enqueueEntityComponentChange(this, componentType);
        }
        return has(componentType);
    }

    protected void removeComponentInternal(ComponentType componentType) {
        boolean removed = components.get(componentType.getIndex()) != null;
        components.remove(componentType.getIndex());
        if(removed)ecs.enqueueEntityComponentEvent(this, componentType, false);
    }

    protected void addToPoolInternal(EntityPool entityPool) {
        poolIndices.add(entityPool.getPoolIndex());
    }

    protected void removeFromPoolInternal(EntityPool entityPool) {
        poolIndices.remove(entityPool.getPoolIndex());
    }

    /**
     * @return unique index for each entity
     */
    public int getEntityIndex() {
        return entityIndex;
    }


    public Entity createChild(){
        Entity entity = ecs.createEntity();
        entity.parent = entityIndex;
        children.add(entity.getEntityIndex());
        return entity;
    }

    public Entity getParent(){
        if(parent == -1)return null;
        return ecs.getEntity(entityIndex);
    }

    public boolean isChildOf(Entity entity){
        return parent==entity.getEntityIndex();
    }

    public boolean hasChild(Entity entity){
        return children.contains(entity.getEntityIndex());
    }

    public Iterable<Entity> children(){
        return () -> new Iterator<>() {

            private Iterator<Integer> indicesIterator = Entity.this.children.iterator();

            @Override
            public boolean hasNext() {
                return indicesIterator.hasNext();
            }

            @Override
            public Entity next() {
                return ecs.getEntity(indicesIterator.next());
            }
        };
    }

    public void relocate(Entity entity){
        if(parent != -1){
            Entity parentEntity = ecs.getEntity(parent);
            parentEntity.children.remove(entityIndex);
        }
        parent = entity.getEntityIndex();
        entity.children.add(entityIndex);
    }

    protected void respawn(int entityIndex) {
        this.entityIndex = entityIndex;
        alive = true;
    }

    protected void kill() {
        if (parent != -1) {
            Entity parentEntity = ecs.getEntity(parent);
            parentEntity.children.remove(entityIndex);
            parent = -1;
        }
        entityIndex = -1;
        componentAddedObservers.clear();
        componentRemovedObserver.clear();
        components.clear();
        children.clear();
        alive = false;
    }

    protected boolean isAlive() {
        return alive;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Entity entity = (Entity) o;
        return entityIndex == entity.entityIndex;
    }

    @Override
    public int hashCode() {
        return Objects.hash(entityIndex);
    }

    public String toString(){
        return "Entity:"+components;
    }

    public void addComponentAddedObserver(ComponentObserver observer){
        componentAddedObservers.add(observer);
    }

    public void addComponentRemovedObserver(ComponentObserver observer){
        componentRemovedObserver.add(observer);
    }

    public void removeComponentAddedObserver(ComponentObserver observer){
        componentAddedObservers.remove(observer);
    }

    public void removeComponentRemovedObserver(ComponentObserver observer){
        componentRemovedObserver.remove(observer);
    }

    protected void notifyObservers(ComponentType componentType, boolean added) {
        if(added){
            for(ComponentObserver observer : componentAddedObservers){
                observer.accept(this, componentType.getComponentClass());
            }
        }else{
            for(ComponentObserver observer : componentRemovedObserver){
                observer.accept(this, componentType.getComponentClass());
            }
        }
    }
}
