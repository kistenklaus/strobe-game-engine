package org.strobe.engine.development.inspector;

import imgui.ImGui;
import org.strobe.ecs.Component;
import org.strobe.ecs.Entity;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class DefaultComponentInspectorNode extends ComponentInspectorNode<Component>{

    @Override
    public Class<? extends Component>[] getInspectorTargets() {
        return new Class[0];
    }

    @Override
    public void drawInspectorNode(Graphics gfx, DevelopmentStyle style, Component component, Entity entity) {
        ImGui.text(component.getClass().getSimpleName());
    }
}
