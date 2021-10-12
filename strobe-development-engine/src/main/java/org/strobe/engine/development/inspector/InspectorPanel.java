package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiInputTextFlags;
import imgui.type.ImString;
import org.strobe.ecs.Component;
import org.strobe.ecs.Entity;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.engine.development.HierarchyPanel;
import org.strobe.engine.development.Panel;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.window.imgui.ImGuiUtils;

import java.util.HashMap;
import java.util.ServiceLoader;

public final class InspectorPanel extends Panel {

    private final HierarchyPanel hierarchy;
    private final DevelopmentStyle style;

    private final HashMap<Class<? extends Component>, ComponentInspector> componentInspectors = new HashMap<>();
    private final ComponentInspector<Component> defaultInspector = new DefaultComponentInspector();

    private final ImString entityNameStr = new ImString(100);

    public InspectorPanel(HierarchyPanel hierarchyPanel, DevelopmentStyle style) {
        this.hierarchy = hierarchyPanel;
        this.style = style;

        ServiceLoader<ComponentInspector> serviceLoader = ServiceLoader.load(ComponentInspector.class);
        serviceLoader.stream().forEach(p -> {
            for (Class<? extends Component> targetClass : p.get().getInspectorTargets()) {
                componentInspectors.put(targetClass, p.get());
            }
        });
    }

    @Override
    public void init(Graphics gfx) {
    }

    @Override
    @SuppressWarnings("unchecked")
    public void draw(Graphics gfx) {
        ImGui.begin("Inspector");
        final Entity entity = hierarchy.getSelectedEntity();


        if (entity != null) {
            Texture2D entityIcon = style.getEntityIcon(entity);

            ImVec2 cp = ImGui.getCursorPos();

            ImGuiUtils.image(entityIcon.getID(), 50, 50);

            ImGui.setCursorPos(cp.x + 60, cp.y + 4);


            entityNameStr.set(entity.getName(), false);

            ImGui.pushItemWidth(200);
            boolean confirm = ImGui.inputText(" ", entityNameStr, ImGuiInputTextFlags.EnterReturnsTrue
                    | ImGuiInputTextFlags.CallbackAlways | ImGuiInputTextFlags.CallbackResize
                    | ImGuiInputTextFlags.CharsNoBlank);
            ImGui.popItemWidth();

            if (confirm && !entityNameStr.isEmpty()) {
                entity.setName(entityNameStr.get());
            }

            ImGui.setCursorPos(cp.x, cp.y + 60);

            for (Component component : entity.components()) {
                ImGui.separator();
                ComponentInspector inspector = componentInspectors.get(component.getClass());
                if (inspector != null) {
                    inspector.drawInspector(gfx, style, component, entity);
                } else {
                    defaultInspector.drawInspector(gfx, style, component, entity);
                }
            }
        } else {
            ImGui.separator();
            ImGui.text("no entity selected");
        }
        ImGui.separator();
        ImGui.end();
    }
}
