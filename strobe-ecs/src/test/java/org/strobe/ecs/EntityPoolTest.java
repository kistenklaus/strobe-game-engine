package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.AnotherComponent;
import org.strobe.ecs.testcomponents.OtherComponent;
import org.strobe.ecs.testcomponents.ParentComponent;

public final class EntityPoolTest {

    private static final float DT = 0.1f;

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        this.ecs = new EntityComponentSystem();
        ecs.disableInstantUpdate();
    }

    @Test
    void poolContainsTesting(){
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        Entity entity = ecs.createEntity("abc");
        entity.addComponent(new OtherComponent());
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));
        ecs.update(DT);
        Assertions.assertEquals(1, pool.getSize());
        Assertions.assertTrue(pool.contains(entity));

        entity.removeComponent(OtherComponent.class);
        Assertions.assertEquals(1, pool.getSize());
        Assertions.assertTrue(pool.contains(entity));
        ecs.update(DT);
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));

        entity.addComponent(new AnotherComponent());
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));
        ecs.update(DT);
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));


        //add, remove testing
        entity.addComponent(new OtherComponent());
        entity.removeComponent(OtherComponent.class);
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));
        ecs.update(DT);
        Assertions.assertEquals(0, pool.getSize());
        Assertions.assertFalse(pool.contains(entity));


        entity.addComponent(new OtherComponent());
        ecs.update(DT);
        Assertions.assertTrue(pool.contains(entity));
        Assertions.assertEquals(1, pool.getSize());
        //remove, add testing
        entity.removeComponent(OtherComponent.class);
        entity.addComponent(new OtherComponent());
        ecs.update(DT);
        Assertions.assertEquals(1, pool.getSize());
        Assertions.assertTrue(pool.contains(entity));
    }

    @Test
    void poolDuplication(){
        EntityPool p1 = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        EntityPool p2 = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        Assertions.assertTrue(p1 == p2);
    }

    @Test
    void requireAllPoolTest(){
        EntityPool p1 = ecs.createPool(EntityFilter.requireAll(OtherComponent.class, AnotherComponent.class));
        Entity ent = ecs.createEntity("ent");
        ent.addComponent(new OtherComponent());
        ent.addComponent(new AnotherComponent());
        ecs.update(DT);
        Assertions.assertTrue(p1.getSize() == 1);
    }

    @Test
    void latePoolCreation(){
        Entity e = ecs.createEntity("e");
        e.addComponent(new OtherComponent());
        ecs.update(0);
        EntityPool pool = ecs.createPool(EntityFilter.requireAll(OtherComponent.class));
        ecs.update(0);
        Assertions.assertTrue(pool.getSize() != 0);
    }
}
