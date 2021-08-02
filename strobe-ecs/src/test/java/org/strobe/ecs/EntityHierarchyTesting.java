package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public final class EntityHierarchyTesting {

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
        ecs.disableInstantUpdate();
    }

    @Test
    void testEntityHierarchy(){
        Entity parent = ecs.createEntity();

        Entity child = parent.createChild();
        Assertions.assertTrue(parent.hasChild(child));
        Assertions.assertTrue(child.isChildOf(parent));

        Entity other = ecs.createEntity();
        child.relocate(other);

        Assertions.assertFalse(parent.hasChild(child));
        Assertions.assertTrue(other.hasChild(child));
        Assertions.assertTrue(child.isChildOf(other));
        Assertions.assertFalse(child.isChildOf(parent));
        Assertions.assertTrue(child.getParent().equals(other));


    }
}
