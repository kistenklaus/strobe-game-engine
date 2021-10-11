package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.*;

public class EntityFilterTest {

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
        ecs.enableInstantUpdate();
        ComponentType.resetComponentTypes();
    }

    @Test
    void entityFilterEqualsTest(){
        EntityFilter filter1 = EntityFilter.requireAll(ParentComponent.class, OtherComponent.class);
        EntityFilter filter2 = EntityFilter.requireAll(OtherComponent.class, ParentComponent.class);
        Assertions.assertEquals(filter1, filter2);
        Assertions.assertEquals(filter2, filter1);
        Assertions.assertEquals(filter1.hashCode(), filter2.hashCode());

        filter2 = EntityFilter.requireAll(OtherComponent.class, ParentComponent.class, AnotherComponent.class);
        Assertions.assertNotEquals(filter1, filter2);
        Assertions.assertNotEquals(filter2, filter1);

        filter1 = EntityFilter.requireAll(ParentComponent.class, OtherComponent.class, ChildComponent.class);
        Assertions.assertNotEquals(filter1, filter2);
        Assertions.assertNotEquals(filter2, filter1);

        EntityFilter comp1 = EntityFilter.requireAll(filter2, filter1);
        EntityFilter comp2 = EntityFilter.requireAll(filter1, filter2);
        Assertions.assertEquals(comp1, comp2);
        Assertions.assertEquals(comp2, comp1);
        Assertions.assertEquals(comp1.hashCode(), comp2.hashCode());

        comp2 = EntityFilter.requireAll(filter1);
        Assertions.assertNotEquals(comp1, comp2);
        Assertions.assertNotEquals(comp2, comp1);

        comp1 = EntityFilter.requireAll(filter1, filter1);
        comp2 = EntityFilter.requireAll(filter1, filter2);
        Assertions.assertNotEquals(comp1, comp2);
        Assertions.assertNotEquals(comp2, comp1);
    }

    @Test
    void testSensitivity(){
        EntityFilter filter = EntityFilter.requireAll(OtherComponent.class);
        Assertions.assertTrue(filter.isSensitive(OtherComponent.class));
        Assertions.assertFalse(filter.isSensitive(AnotherComponent.class));

        filter = EntityFilter.requireAll(ChildChildComponent.class);
        Assertions.assertTrue(filter.isSensitive(ParentComponent.class));
        Assertions.assertTrue(filter.isSensitive(ChildComponent.class));
        Assertions.assertFalse(filter.isSensitive(OtherComponent.class));
    }

    @Test
    void testHashCode(){
        EntityFilter f1 = EntityFilter.requireAll(OtherComponent.class);
        EntityFilter f2 = EntityFilter.requireAll(OtherComponent.class);
        Assertions.assertEquals(f1.hashCode(), f2.hashCode());
    }

    @Test
    void requireAllFilterTest(){
        EntityFilter f1 = EntityFilter.requireAll(OtherComponent.class, AnotherComponent.class);
        Entity e = ecs.createEntity("e");
        Assertions.assertFalse(f1.eval(e));
        e.addComponent(new OtherComponent());
        Assertions.assertFalse(f1.eval(e));
        e.addComponent(new AnotherComponent());
        Assertions.assertTrue(f1.eval(e));
    }
}
