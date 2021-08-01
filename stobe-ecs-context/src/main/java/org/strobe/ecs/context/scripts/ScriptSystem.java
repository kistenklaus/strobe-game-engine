package org.strobe.ecs.context.scripts;

import org.strobe.ecs.*;

import java.util.HashMap;
import java.util.Map;

public final class ScriptSystem extends EntitySystem {

    private static final ComponentMapper<ScriptComponent> SCRIPT = ComponentMapper.getFor(ScriptComponent.class);

    private final EntityPool pool;

    private final Map<Entity, ScriptComponent> scripts = new HashMap<>();

    public ScriptSystem(EntityComponentSystem ecs) {
        super(ecs);
        pool = ecs.createPool(EntityFilter.requireAll(ScriptComponent.class));
        pool.addEntityAddedObserver(this::onAddScript);
        pool.addEntityRemovedObserver(this::onRemoveScript);
        for (Entity entity : pool) {
            onAddScript(entity);
        }
    }

    private void onAddScript(Entity entity) {
        if(scripts.containsKey(entity))
            throw new IllegalStateException("a script can only be associated with one entity");
        ScriptComponent script = entity.get(SCRIPT);
        script.link(entity);
        scripts.put(entity, script);
        script.awake();
    }

    private void onRemoveScript(Entity entity) {
        ScriptComponent script = scripts.remove(entity);
        if (script == null) throw new IllegalStateException();
        script.shutdown();
        script.unlink();
    }

    @Override
    public void init() {
    }

    @Override
    public void update(float dt) {
        for (Entity entity : pool) {
            ScriptComponent script = entity.get(SCRIPT);
            if(script.enabled != script.prev_enabled){
                if (script.prev_enabled) script.stop();
                else script.start();
                script.prev_enabled = script.enabled;
            }
            script.update(dt);
        }
    }

    @Override
    public void dispose() {
        pool.removeEntityAddedObserver(this::onAddScript);
        pool.removeEntityRemovedObserver(this::onRemoveScript);
    }
}
