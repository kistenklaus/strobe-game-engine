package org.strobe.engine.development;

import org.strobe.engine.StrobeContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.window.imgui.ImGuiWindow;

public final class DevelopmentEngine extends StrobeEngine {

    public DevelopmentEngine(StrobeContext context) {
        super(context, new ImGuiWindow(context.getTitle(), context.getWidth(), context.getHeight()));
    }


}
