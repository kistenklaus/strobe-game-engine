package org.strobe.debug.imgui;

import imgui.*;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiConfigFlags;
import imgui.gl3.ImGuiImplGl3;
import imgui.glfw.ImGuiImplGlfw;
import org.strobe.utils.ResourceLoader;
import org.strobe.window.Window;

import java.io.IOException;
import java.nio.ByteBuffer;

import static org.lwjgl.glfw.GLFW.*;

final class ImGuiLayer {

    private final Window window;
    private final ImGuiImplGl3 imGuiGl3 = new ImGuiImplGl3();
    private final ImGuiImplGlfw imGuiGlfw = new ImGuiImplGlfw();

    public ImGuiLayer(Window window) {
        this.window = window;

        ImGui.createContext();

        final ImGuiIO io = ImGui.getIO();

        ImFontAtlas atlas = io.getFonts();
        ImFontConfig config = new ImFontConfig();
        config.setGlyphRanges(atlas.getGlyphRangesDefault());
        config.setMergeMode(false);
        config.setPixelSnapH(false);

        ByteBuffer buffer = null;
        try {
            buffer = ResourceLoader.ioResourceToByteBuffer("fonts/JetBrainsMono/JetBrainsMono-SemiBold.ttf");
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] array = new byte[buffer.capacity()];
        buffer.position(0);
        for (int i = 0; i < buffer.capacity(); i++) array[i] = buffer.get();
        atlas.addFontFromMemoryTTF(array, 16.0f);

        io.addConfigFlags(ImGuiConfigFlags.NavEnableKeyboard);
        io.addConfigFlags(ImGuiConfigFlags.DockingEnable);      // Enable Docking
        io.addConfigFlags(ImGuiConfigFlags.ViewportsEnable);
        io.setConfigViewportsNoTaskBarIcon(true);

        io.getConfigViewportsNoDecoration();
        io.getConfigViewportsNoDefaultParent();

        if (io.hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
            final ImGuiStyle style = ImGui.getStyle();
            style.setWindowRounding(0.0f);
            style.setColor(ImGuiCol.WindowBg, ImGui.getColorU32(ImGuiCol.WindowBg, 1));
        }

        imGuiGlfw.init(window.pointer(), true);

        imGuiGl3.init("#version 420 core");

        startFrame();
        endFrame();
    }

    public void startFrame() {
        imGuiGlfw.newFrame();
        ImGui.newFrame();
    }

    public void endFrame() {
        ImGui.render();
        imGuiGl3.renderDrawData(ImGui.getDrawData());
        if (ImGui.getIO().hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
            final long backupWindowPtr = glfwGetCurrentContext();
            ImGui.updatePlatformWindows();
            ImGui.renderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupWindowPtr);
        }
    }

    public void dispose() {
        imGuiGl3.dispose();
        imGuiGlfw.dispose();
        ImGui.destroyContext();
    }

}
