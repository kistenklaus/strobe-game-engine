package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.AnotherComponent;
import org.strobe.ecs.testcomponents.ChildComponent;
import org.strobe.ecs.testcomponents.OtherComponent;
import org.strobe.ecs.testcomponents.ParentComponent;

public final class EntityTest {

    private static final float DT = 0.1f;

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach() {
        ComponentType.resetComponentTypes();
        ecs = new EntityComponentSystem();
    }


    @Test
    void hasTest(){
        Entity entity = ecs.createEntity();
        entity.addComponent(new OtherComponent());
        entity.addComponent(new ChildComponent());
        ecs.update(DT);
        Assertions.assertTrue(entity.has(OtherComponent.class));
        Assertions.assertTrue(entity.has(ChildComponent.class));
        Assertions.assertTrue(entity.has(ParentComponent.class));
        Assertions.assertFalse(entity.has(AnotherComponent.class));
    }

    @Test
    void removeTest(){
        Entity entity = ecs.createEntity();
        entity.addComponent(new OtherComponent());
        entity.addComponent(new ChildComponent());
        ecs.update(DT);
        entity.removeComponent(OtherComponent.class);
        ecs.update(DT);
        Assertions.assertTrue(entity.has(ChildComponent.class));
        Assertions.assertFalse(entity.has(OtherComponent.class));

        entity.addComponent(new OtherComponent());
        //--
        entity.removeComponent(OtherComponent.class);
        ecs.update(DT);
        Assertions.assertTrue(entity.has(ChildComponent.class));
        Assertions.assertFalse(entity.has(OtherComponent.class));

    }

}
