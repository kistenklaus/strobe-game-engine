package org.strobe.game;

import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.Mesh;
import org.strobe.window.WindowKey;
import org.strobe.window.WindowKeyboard;

public final class TestMeshSystem extends EntitySystem {

    private final WindowKeyboard keyboard;

    private final EntityPool entityPool;

    public TestMeshSystem(EntityComponentSystem ecs, WindowKeyboard keyboard) {
        super(ecs);
        this.keyboard = keyboard;
        entityPool = ecs.createPool(EntityFilter.requireAll(Mesh.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for (Entity entity : entityPool) {
            Mesh mesh = entity.get(Mesh.class);
            if (keyboard.isKeyDown(WindowKey.KEY_V)) {
                mesh.setPositions(0, new float[]{-1, -1, 0,  -1, 1, 0,  1, 1, 0,  1, -1, 0});
            }else{
                mesh.setPositions(0, new float[]{-0.5f, -0.5f, 0f, -0.5f, 0.5f, 0f, 0.5f, 0.5f, 0f, 0.5f, -0.5f, 0f});
            }
        }
    }

    @Override
    public void dispose() {

    }
}
