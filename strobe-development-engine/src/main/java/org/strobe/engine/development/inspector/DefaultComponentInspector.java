package org.strobe.engine.development.inspector;

import org.strobe.ecs.Component;
import org.strobe.ecs.Entity;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class DefaultComponentInspector extends ComponentInspector<Component> {

    public DefaultComponentInspector(){
        super(true);
    }

    @Override
    public Class<? extends Component>[] getInspectorTargets() {
        return new Class[0];
    }

    @Override
    public void drawInspectorNode(Graphics gfx, DevelopmentStyle style, Component component, Entity entity) {
    }
}
