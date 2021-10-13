package org.strobe.engine.development;

import imgui.ImGui;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.engine.development.ui.*;
import org.strobe.gfx.Graphics;
import org.strobe.window.WindowContentRegion;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.imgui.ImGuiWindow;

public final class DevelopmentEngine extends StrobeEngine<EntityContext> {

    private final DevelopmentUIRenderer uiRenderer = new DevelopmentUIRenderer();
    private final ProxyPrintStream proxyPrintStream = new ProxyPrintStream();
    private final WindowContentRegion contentRegion = new WindowContentRegion(0, 0, 1, 1);

    public DevelopmentEngine(EntityContext context) {
        super(context, new ImGuiWindow(context.getTitle(), "development.ini", context.getWidth(), context.getHeight(),
                WindowConfiguration.get(false, false, true, true, false),
                "fonts/JetBrainsMono/JetBrainsMono-SemiBold.ttf"));
    }

    @Override
    public void afterInit(Graphics gfx) {
        gfx.getWindow().setContentRegion(contentRegion);


        uiRenderer.addUI(new ViewportPanel(context, contentRegion));
        HierarchyPanel hierarchy;
        uiRenderer.addUI(hierarchy = new HierarchyPanel(context, uiRenderer.getStyle()));
        uiRenderer.addUI(new InspectorPanel(hierarchy, uiRenderer.getStyle()));
        uiRenderer.addUI(new ConsolePanel(proxyPrintStream, uiRenderer.getStyle()));
        uiRenderer.addUI(new ProfilerPanel(uiRenderer.getStyle()));

        uiRenderer.init(gfx);
        gfx.addRenderer(0, uiRenderer);
    }


}
