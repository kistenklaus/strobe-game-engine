package org.strobe.engine.release;

import org.strobe.engine.StrobeContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.glfw.GlfwWindow;

public final class ReleaseEngine extends StrobeEngine {

    public ReleaseEngine(StrobeContext context) {
        super(context, new GlfwWindow(context.getTitle(), context.getWidth(), context.getHeight(),
                WindowConfiguration.get()));
    }
}
