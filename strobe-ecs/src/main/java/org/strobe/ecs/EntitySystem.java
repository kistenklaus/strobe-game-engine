package org.strobe.ecs;

public abstract class EntitySystem {

    protected EntityComponentSystem ecs;
    private boolean enabled = true;

    public EntitySystem(EntityComponentSystem ecs){
        this.ecs = ecs;
    }

    public abstract void init();

    public abstract void update(float dt);

    public abstract void dispose();

    public boolean isEnabled(){
        return enabled;
    }

    public void disable(){
        enabled = false;
    }

    public void enable(){
        enabled = true;
    }

}
