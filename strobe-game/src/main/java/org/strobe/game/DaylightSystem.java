package org.strobe.game;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.window.WindowKey;
import org.strobe.window.WindowKeyboard;

public final class DaylightSystem extends EntitySystem {

    private final EntityPool pool;
    private final WindowKeyboard keyboard;

    private float cycle = 0.1f;

    public DaylightSystem(EntityComponentSystem ecs, WindowKeyboard keyboard) {
        super(ecs);
        this.keyboard = keyboard;
        pool = ecs.createPool(EntityFilter.requireAll(EntityFilter.requireAll(DaylightCycle.class, Transform.class)));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        if(keyboard.isKeyDown(WindowKey.KEY_V))cycle += dt * 0.75f;
        for(Entity entity : pool){
            Transform transform = entity.get(Transform.class);
            transform.setPosition((float)Math.cos(cycle)*0, (float) Math.cos(cycle)*3,
                    (float)Math.sin(cycle)*3);
        }
    }

    @Override
    public void dispose() {

    }
}
