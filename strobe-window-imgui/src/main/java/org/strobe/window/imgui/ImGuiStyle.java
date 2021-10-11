package org.strobe.window.imgui;

import org.strobe.gfx.Graphics;

public interface ImGuiStyle {

    String getFontResource();

    void push();

    void pop();
}
