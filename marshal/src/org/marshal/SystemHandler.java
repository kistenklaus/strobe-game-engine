package org.marshal;

import org.marshal.systems.EntitySystem;

import java.util.ArrayList;
import java.util.HashSet;

/**
 * handles all system of the thread the family handler is attached to
 *
 * @author Karl
 * @version 3.0
 */
class SystemHandler {

    private ArrayList<EntitySystem> systems = new ArrayList<>();
    private HashSet<EntitySystem> systemSet = new HashSet<>();

    private long lastNano;
    private boolean updateSystems = false;

    private final EntityComponentSystem ecs;

    public SystemHandler(EntityComponentSystem ecs){
        this.ecs = ecs;
    }

    public void startSystems() {
        lastNano = System.nanoTime();
        updateSystems = true;
    }

    public void updateSystems() {
        if (!updateSystems) return;
        long curr = System.nanoTime();
        float dt = (curr - lastNano) * 1e-9f;
        lastNano = curr;
        for (EntitySystem system : systems) {
            system.update(dt);
        }
    }

    public void addSystem(EntitySystem system) {
        systems.add(system);
        systemSet.add(system);
        system.connect(ecs);
    }

    public boolean containsSystem(EntitySystem system) {
        return systemSet.contains(system);
    }

    public void removeSystem(EntitySystem system) {
        if (systemSet.contains(system)) {
            systems.remove(system);
            systemSet.remove(system);
            system.disconnect();
        }


    }
}
