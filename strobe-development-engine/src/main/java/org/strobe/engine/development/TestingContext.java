package org.strobe.engine.development;

import imgui.ImGui;
import imgui.flag.ImGuiCond;
import imgui.flag.ImGuiWindowFlags;
import imgui.type.ImBoolean;
import org.strobe.engine.StrobeContext;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL11.GL_COLOR_BUFFER_BIT;
import static org.lwjgl.opengl.GL11.glClear;

public final class TestingContext extends StrobeContext {

    public static void main(String[] args) {
        StrobeContext context = new TestingContext();
        new DevelopmentEngine(context);
        context.start();
    }


    public TestingContext() {
        super("testing", 640, 480);
    }

    @Override
    public void init(Graphics gfx) {

    }

    @Override
    public void render(Graphics gfx) {
        ImGui.begin("testing");

        ImGui.end();
    }

    @Override
    public void update(float dt) {

    }
}
