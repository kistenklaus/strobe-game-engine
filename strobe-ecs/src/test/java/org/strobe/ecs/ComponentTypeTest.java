package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.ChildChildComponent;
import org.strobe.ecs.testcomponents.ChildComponent;
import org.strobe.ecs.testcomponents.ParentComponent;

public class ComponentTypeTest {


    @BeforeEach
    void beforeEach(){
        ComponentType.resetComponentTypes();
    }

    @Test
    void hierarchyTestingChildFirst(){
        ComponentType childChildType = ComponentType.getFor(ChildChildComponent.class);

        ComponentType childType = ComponentType.getFor(ChildComponent.class);

        ComponentType parentType = ComponentType.getFor(ParentComponent.class);

        Assertions.assertEquals(childChildType, childType);
        Assertions.assertEquals(childType, parentType);
        Assertions.assertEquals(parentType, childChildType);
    }

    @Test
    void hierarchyTestingParentFirst(){

    }

}
