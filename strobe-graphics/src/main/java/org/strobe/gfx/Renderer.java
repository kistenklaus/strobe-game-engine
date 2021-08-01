package org.strobe.gfx;

public interface Renderer<T extends Drawable>{

    void draw(Graphics gfx, T drawable);

    void render(Graphics gfx);

    void dispose(Graphics gfx);
}
