package org.strobe.engine.development.inspector;

import imgui.ImGui;
import org.strobe.ecs.Component;
import org.strobe.ecs.ComponentType;
import org.strobe.ecs.Entity;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;
import org.strobe.utils.UnboundedArray;
import org.strobe.window.imgui.ImGuiUtils;

import java.util.BitSet;

public abstract class ComponentInspectorNode<T extends Component> {

    private double lastPress = 0.0f;
    private final UnboundedArray<BitSet> openState = new UnboundedArray<>(8);
    private Entity currentEntity = null;

    public abstract Class<? extends T>[] getInspectorTargets();

    public final void drawInspector(Graphics gfx, DevelopmentStyle style, T component, Entity entity){
        BitSet os = openState.get(entity.getEntityIndex());
        if(os == null){
            os = new BitSet();
            openState.set(entity.getEntityIndex(), os);
        }
        int componentIndex = ComponentType.getIndexFor(component.getClass());

        if(ImGuiUtils.iconTreeNodeButton(ImGui.getID(entity.getEntityIndex()+component.getClass().getSimpleName()),
                getDisplayName(component), -1,20,
                os.get(componentIndex),
                true, style.getDarkColor(),
                style.getOpenTreeIcon().getID(), style.getClosedTreeIcon().getID(),
                getDisplayIcon(component, style))){
            double current = ImGui.getTime();
            double delta = current - lastPress;
            lastPress = current;
            if(delta < 0.25f){
                os.flip(componentIndex);
            }
        }
        ImGui.pushID(entity.getEntityIndex()+component.getClass().getSimpleName());
        if(os.get(componentIndex))drawInspectorNode(gfx,style, component, entity);
        ImGui.popID();
    }

    public String getDisplayName(T component){
        return component.getClass().getSimpleName();
    }

    public int getDisplayIcon(T component, DevelopmentStyle style){
        return style.getRenderableEntityIcon().getID();
    }

    protected abstract void drawInspectorNode(Graphics gfx, DevelopmentStyle style, T component, Entity entity);

}
