package org.strobe.engine.development.profiler.ecs;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.engine.development.profiler.Profiler;

public class ProfileableEntityComponentSystem extends EntityComponentSystem {

    private final Profiler profiler;

    public ProfileableEntityComponentSystem(Profiler profiler){
        this.profiler = profiler;
    }

    @Override
    public void update(float dt) {
        profiler.pushFrame("ecs-update");
        super.update(dt);
        profiler.popFrame();
    }

    @Override
    protected void processSystemOperations() {
        profiler.pushFrame("process-system-operations");
        super.processSystemOperations();
        profiler.popFrame();
    }

    @Override
    protected void executeSystems(float dt) {
        profiler.pushFrame("execute-system");
        super.executeSystems(dt);
        profiler.popFrame();
    }

    @Override
    protected void processEntityOperations() {
        profiler.pushFrame("process-entity-operations");
        super.processEntityOperations();
        profiler.popFrame();
    }

    @Override
    protected void processEntityPoolChanges() {
        profiler.pushFrame("process-entity-pool-changes");
        super.processEntityPoolChanges();
        profiler.popFrame();
    }

    @Override
    protected void processEntityPoolEvents() {
        profiler.pushFrame("process-entity-pool-events");
        super.processEntityPoolEvents();
        profiler.popFrame();
    }

    @Override
    protected void processEntityComponentEvents() {
        profiler.pushFrame("entity-component-events");
        super.processEntityComponentEvents();
        profiler.popFrame();
    }
}
