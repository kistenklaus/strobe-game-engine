package org.strobe.ecs;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.ecs.testcomponents.OtherComponent;

public final class ComponentMapperTest {

    private EntityComponentSystem ecs;

    @BeforeEach
    void beforeEach(){
        ecs = new EntityComponentSystem();
        ecs.enableInstantUpdate();
    }

    @Test
    void componentMapperTesting(){
        Entity entity = ecs.createEntity("entity");
        entity.addComponent(new OtherComponent());
    }

}
