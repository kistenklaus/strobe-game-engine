package org.strobe.engine.development.ui;

import imgui.ImGui;
import org.strobe.gfx.Graphics;

public final class ProfilerPanel implements UIPanel {

    private final DevelopmentStyle style;

    public ProfilerPanel(DevelopmentStyle style){
        this.style = style;
    }

    @Override
    public void init(Graphics gfx) {

    }

    @Override
    public void draw(Graphics gfx) {
        if(ImGui.begin("Profiler")){

        }
        ImGui.end();
    }
}
