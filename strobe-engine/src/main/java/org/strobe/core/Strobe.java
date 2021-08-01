package org.strobe.core;

import org.strobe.window.Window;

public final class Strobe {

    public static StrobeContext create(StrobeContext context, Window window){
        new StrobeEngine(context, window);
        return context;
    }

    public static void start(StrobeContext context, Window window) {
        create(context, window).start();
    }
}
