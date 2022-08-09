package org.marshal.systems;

public abstract class IntervalSystem extends EntitySystem {

    private float interval = 1/60f;

    private float intervalTimer = 0;

    public IntervalSystem(){
        super();
    }

    public IntervalSystem(float interval){
        super();
        this.interval = interval;
    }

    public final void update(float dt){
        intervalTimer += dt;
        if(interval == 0.0f){
            interval(intervalTimer);
            intervalTimer = 0;
            return;
        }
        while(intervalTimer>=interval) {
            interval(interval);
            intervalTimer-=interval;
        }
    }

    public abstract void interval(float dt);

    public final void setInterval(float interval){
        this.interval = interval;
    }

    public final float getInterval(){
        return this.interval;
    }

}
