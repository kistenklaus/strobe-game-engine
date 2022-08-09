package org.marshal.systems;


import org.marshal.EntityComponentSystem;

/**
 * An EntitySystem is the simplest way to implement logic into the ecs.
 * an system is getting updated every time the ecs is updated (on the thread the system was added)
 * only after the ecs.startSystems() is called()
 * @version 3.0
 * @author Karl
 */
public abstract class EntitySystem {

    private EntityComponentSystem ecs;

    public void connect(EntityComponentSystem ecs){
        this.ecs = ecs;
        start();
    }

    public abstract void start();

    /**
     * updates when the ecs is updated
     * @param dt time since the last update
     */
    public abstract void update(float dt);


    public abstract void stop();

    public void disconnect(){
        stop();
        this.ecs = null;
    }

    public EntityComponentSystem getEcs(){
        return ecs;
    }
}
