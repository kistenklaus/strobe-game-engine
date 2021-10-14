package org.strobe.engine.development;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.engine.development.profiler.Profiler;
import org.strobe.engine.development.profiler.ecs.ProfileableEntityComponentSystem;
import org.strobe.engine.development.profiler.ecs.ProfileableImGuiWindow;
import org.strobe.engine.development.ui.*;
import org.strobe.gfx.Graphics;
import org.strobe.window.WindowContentRegion;
import org.strobe.window.WindowConfiguration;

public final class DevelopmentEngine extends StrobeEngine<EntityContext> {

    private final DevelopmentUIRenderer uiRenderer = new DevelopmentUIRenderer();
    private final ProxyPrintStream proxyPrintStream = new ProxyPrintStream();
    private final WindowContentRegion contentRegion = new WindowContentRegion(0, 0, 1, 1);

    private final Profiler profiler;

    public DevelopmentEngine(EntityContext context, Profiler profiler) {
        super(context, new ProfileableImGuiWindow(context.getTitle(), "development.ini", context.getWidth(), context.getHeight(),
                WindowConfiguration.get(false, false, true, true, false),
                "fonts/JetBrainsMono/JetBrainsMono-SemiBold.ttf",profiler));
        this.profiler = profiler;
        context.linkEntityComponentSystem(new ProfileableEntityComponentSystem(profiler));
    }


    @Override
    public void afterInit(Graphics gfx) {
        gfx.getWindow().setContentRegion(contentRegion);


        uiRenderer.addUI(new ViewportPanel(context, contentRegion));
        HierarchyPanel hierarchy;
        uiRenderer.addUI(hierarchy = new HierarchyPanel(context, uiRenderer.getStyle()));
        uiRenderer.addUI(new InspectorPanel(hierarchy, uiRenderer.getStyle()));
        uiRenderer.addUI(new ConsolePanel(proxyPrintStream, uiRenderer.getStyle()));
        uiRenderer.addUI(new ProfilerPanel(uiRenderer.getStyle(), profiler));
        uiRenderer.addUI(new DemoPanel());

        uiRenderer.init(gfx);
        gfx.addRenderer(0, uiRenderer);
        gfx.addRenderer(1, new DevelopmentRenderer(gfx, context, profiler));
    }

    @Override
    protected void loop(float dt) {
        profiler.startFrame();
        profiler.pushFrame("window-new-frame");
        gfx.newFrame();
        profiler.popFrame();

        profiler.pushFrame("context-render");
        context.render(gfx);
        profiler.popFrame();
        //render

        profiler.pushFrame("engine-before-render");
        beforeRender(gfx);
        profiler.popFrame();
        profiler.pushFrame("gfx-render");
        gfx.render();
        profiler.popFrame();
        profiler.pushFrame("engine-after-render");
        afterRender(gfx);
        profiler.popFrame();
        //context render (submit)
        //render debug
        //logic
        profiler.pushFrame("engine-before-render");
        beforeUpdate(dt);
        profiler.popFrame();
        profiler.pushFrame("context-update");
        context.update(dt);
        profiler.popFrame();
        profiler.pushFrame("engine-after-update");
        afterUpdate(dt);
        profiler.popFrame();

        //swapping buffers (syncing of gpu and cpu)
        profiler.pushFrame("engine-before-swap-buffers");
        profiler.popFrame();
        profiler.pushFrame("gfx-end-frame");
        gfx.endFrame();
        profiler.popFrame();
        profiler.endFrame();
    }

}
