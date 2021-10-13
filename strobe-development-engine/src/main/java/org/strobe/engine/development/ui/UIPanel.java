package org.strobe.engine.development.ui;

import org.strobe.gfx.Drawable;
import org.strobe.gfx.Graphics;

interface UIPanel extends Drawable {

    void init(Graphics gfx);

    void draw(Graphics gfx);

    default void dispose(Graphics gfx){}

}
