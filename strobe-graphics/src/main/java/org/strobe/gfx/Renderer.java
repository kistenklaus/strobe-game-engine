package org.strobe.gfx;

public interface Renderer<T extends Drawable>{

    void render(Graphics gfx);

    void dispose(Graphics gfx);
}
