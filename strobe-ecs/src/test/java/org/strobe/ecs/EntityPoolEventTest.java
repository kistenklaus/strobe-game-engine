package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.OtherComponent;

public final class EntityPoolEventTest {

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
    }

    @Test
    void addEventTesting(){
        ecs.disableInstantUpdate();
        Entity entity = ecs.createEntity();
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        final boolean[] received = {false};
        EntityPoolObserver addObserver;
        pool.addEntityAddedObserver(addObserver = entity1 -> received[0] = true);
        entity.addComponent(new OtherComponent());
        ecs.update(0.1f);
        Assertions.assertTrue(received[0]);

        received[0] = false;
        entity.removeComponent(OtherComponent.class);
        ecs.update(0.1f);
        Assertions.assertFalse(received[0]);

        pool.removeEntityAddedObserver(addObserver);
        entity.addComponent(new OtherComponent());
        ecs.update(0.1f);
        Assertions.assertFalse(received[0]);
    }

    @Test
    void removeEventTesting(){
        ecs.disableInstantUpdate();
        Entity entity = ecs.createEntity();
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        final boolean[] received = {false};
        EntityPoolObserver removeObserver;
        pool.addEntityRemovedObserver(removeObserver = entity1 -> received[0] = true);
        entity.addComponent(new OtherComponent());
        Assertions.assertFalse(received[0]);
        ecs.update(0.1f);
        entity.removeComponent(OtherComponent.class);
        ecs.update(0.1f);
        Assertions.assertTrue(received[0]);
        received[0] = false;

        pool.removeEntityRemovedObserver(removeObserver);

        entity.addComponent(new OtherComponent());
        ecs.update(0.1f);
        entity.removeComponent(OtherComponent.class);
        ecs.update(0.1f);
        Assertions.assertFalse(received[0]);
    }

    @Test
    void chainTesting(){
        ecs.disableInstantUpdate();
        Entity entity = ecs.createEntity();
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));

        pool.addEntityAddedObserver(ent->{
            System.out.println("add event");
        });

        pool.addEntityRemovedObserver(ent->{
            System.out.println("remove event");
        });

        entity.addComponent(new OtherComponent());
        entity.removeComponent(OtherComponent.class);
        ecs.update(0.1f);


    }

}
