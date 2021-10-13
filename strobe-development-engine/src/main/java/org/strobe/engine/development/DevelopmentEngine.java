package org.strobe.engine.development;

import imgui.ImGui;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.engine.development.inspector.InspectorPanel;
import org.strobe.gfx.Graphics;
import org.strobe.window.WindowContentRegion;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.imgui.ImGuiWindow;

import java.io.PrintStream;

public final class DevelopmentEngine extends StrobeEngine<EntityContext> {

    private final WindowContentRegion contentRegion;

    private HierarchyPanel hierarchy;
    private ViewportPanel viewport;
    private InspectorPanel inspector;
    private ConsolePanel console;

    private DevelopmentStyle style;

    public DevelopmentEngine(EntityContext context) {
        super(context, new ImGuiWindow(context.getTitle(), "development.ini", context.getWidth(), context.getHeight(),
                WindowConfiguration.get(false, false, true, true, false),
                new DevelopmentStyle()));
        console = new ConsolePanel((DevelopmentStyle) ((ImGuiWindow)gfx.getWindow()).getStyle());
        contentRegion = new WindowContentRegion(0, 0, 1, 1);


    }

    @Override
    public void afterInit(Graphics gfx) {
        gfx.getWindow().setContentRegion(contentRegion);

        final DevelopmentStyle style = (DevelopmentStyle) ((ImGuiWindow)gfx.getWindow()).getStyle();
        style.init(gfx);
        viewport = new ViewportPanel(context, contentRegion);
        hierarchy = new HierarchyPanel(context, style);
        inspector = new InspectorPanel(hierarchy, style);

        console.init(gfx);
        viewport.init(gfx);
        hierarchy.init(gfx);
        inspector.init(gfx);
    }

    @Override
    protected void beforeRender(Graphics gfx) {
        viewport.draw(gfx);
        hierarchy.draw(gfx);
        inspector.draw(gfx);
        console.draw(gfx);
        ImGui.showDemoWindow();
    }


}
