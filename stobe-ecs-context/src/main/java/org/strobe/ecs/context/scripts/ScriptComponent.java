package org.strobe.ecs.context.scripts;

import org.strobe.ecs.Component;
import org.strobe.ecs.Entity;

public abstract class ScriptComponent implements Component {

    private Entity entity = null;
    boolean enabled = true;
    boolean prev_enabled = false;

    void link(Entity entity){
        this.entity = entity;
    }

    protected void awake() {}

    protected void start(){}

    protected void update(float dt){}

    protected void stop(){}

    protected void shutdown(){}

    void unlink(){
        this.entity = null;
    }

    public final void disable(){
        this.enabled = false;
    }

    public final void enable(){
        this.enabled = true;
    }

    protected final Entity getEntity(){
        return entity;
    }

}
