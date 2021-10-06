package org.strobe.engine;

import org.strobe.debug.Debugger;
import org.strobe.gfx.Graphics;

public abstract class StrobeContext {

    private StrobeEngine engine;

    private final String title;
    private final int width;
    private final int height;

    public StrobeContext(String title, int width, int height) {
        this.width = width;
        this.height = height;
        this.title = title;
    }

    public abstract void init(Graphics gfx);

    public abstract void render(Graphics gfx);

    public abstract void update(float dt);

    public void start() {
        engine.start();
    }

    public void stop() {
        engine.stop();
    }

    final void linkToEngine(StrobeEngine engine) {
        this.engine = engine;
    }

    public int getWidth(){
        return width;
    }

    public int getHeight(){
        return height;
    }

    public String getTitle(){
        return title;
    }

}
