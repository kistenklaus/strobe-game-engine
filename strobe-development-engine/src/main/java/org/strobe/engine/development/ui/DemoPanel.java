package org.strobe.engine.development.ui;

import imgui.ImGui;
import org.strobe.gfx.Graphics;

public class DemoPanel implements UIPanel {
    @Override
    public void init(Graphics gfx) {

    }

    @Override
    public void draw(Graphics gfx) {
        ImGui.showDemoWindow();
    }
}
