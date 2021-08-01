package org.strobe.ecs;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.OtherComponent;

public class ComponentObserverTest {

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
        ecs.enableInstantUpdate();
        ComponentType.resetComponentTypes();
    }

    @Test
    void observerTest(){
        Entity entity = ecs.createEntity();
        boolean[] registerAdd = {false};
        ComponentObserver addObserver = (e,cc)->{
            registerAdd[0] = true;
        };
        entity.addComponentAddedObserver(addObserver);

        boolean[] registerRemove = {false};
        ComponentObserver removeObserver = (e, cc)->{
            registerRemove[0] = true;
        };
        entity.addComponentRemovedObserver(removeObserver);

        Assertions.assertFalse(registerAdd[0]);
        Assertions.assertFalse(registerRemove[0]);

        entity.addComponent(new OtherComponent());
        Assertions.assertTrue(registerAdd[0]);
        Assertions.assertFalse(registerRemove[0]);

        entity.removeComponent(OtherComponent.class);
        Assertions.assertTrue(registerAdd[0]);
        Assertions.assertTrue(registerRemove[0]);

        registerRemove[0] = false;
        registerAdd[0] = false;
        entity.removeComponent(OtherComponent.class);
        Assertions.assertFalse(registerAdd[0]);
        Assertions.assertFalse(registerRemove[0]);

        entity.addComponent(new OtherComponent());
        Assertions.assertTrue(registerAdd[0]);

        registerAdd[0] = false;
        entity.addComponent(new OtherComponent());
        Assertions.assertTrue(registerAdd[0]);
    }

    @Test
    void observerAddAddTest(){
        Entity entity = ecs.createEntity();
        boolean[] registerAdd = {false};
        ComponentObserver addObserver = (e,cc)->{
            registerAdd[0] = true;
            System.out.println("add");
        };
        entity.addComponentAddedObserver(addObserver);

        boolean[] registerRemove = {false};
        ComponentObserver removeObserver = (e, cc)->{
            registerRemove[0] = true;
            System.out.println("remove");
        };
        entity.addComponentRemovedObserver(removeObserver);

        entity.addComponent(new OtherComponent());
        Assertions.assertTrue(registerAdd[0]);
        Assertions.assertFalse(registerRemove[0]);
        registerAdd[0] = false;
        entity.addComponent(new OtherComponent());
        //remove is also invoked when overwriting an existing component
        Assertions.assertTrue(registerAdd[0]);
        Assertions.assertTrue(registerRemove[0]);


    }
}
