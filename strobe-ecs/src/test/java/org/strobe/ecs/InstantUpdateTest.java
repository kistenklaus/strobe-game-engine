package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.OtherComponent;

public final class InstantUpdateTest {
    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
        ecs.enableInstantUpdate();
    }

    @Test
    void instantUpdateTesting(){
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        Entity entity = ecs.createEntity("e");
        entity.addComponent(new OtherComponent());
        Assertions.assertTrue(entity.has(OtherComponent.class));
        Assertions.assertTrue(pool.contains(entity));

        entity.removeComponent(OtherComponent.class);
        Assertions.assertFalse(entity.has(OtherComponent.class));
        Assertions.assertFalse(pool.contains(entity));


    }

}
