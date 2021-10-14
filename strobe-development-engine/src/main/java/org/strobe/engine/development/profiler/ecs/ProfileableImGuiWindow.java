package org.strobe.engine.development.profiler.ecs;

import imgui.ImGui;
import imgui.flag.ImGuiConfigFlags;
import org.strobe.engine.development.profiler.Profiler;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.imgui.ImGuiWindow;

import static org.lwjgl.glfw.GLFW.glfwGetCurrentContext;
import static org.lwjgl.glfw.GLFW.glfwMakeContextCurrent;

public final class ProfileableImGuiWindow extends ImGuiWindow {

    private final Profiler profiler;

    public ProfileableImGuiWindow(String title, String iniFile, int width, int height, WindowConfiguration config, String fontResourcePath,
                                  Profiler profiler) {
        super(title, iniFile, width, height, config, fontResourcePath);
        this.profiler = profiler;
    }


    @Override
    public void swapBuffers() {
        profiler.pushFrame("swap-buffers");
        super.swapBuffers();
        profiler.popFrame();
    }

    @Override
    public void endFrame() {
        endDockSpace();
        profiler.pushFrame("imgui-render");
        ImGui.render();
        imGuiGl3.renderDrawData(ImGui.getDrawData());

        if (ImGui.getIO().hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
            final long backupWindowPtr = glfwGetCurrentContext();
            ImGui.updatePlatformWindows();
            ImGui.renderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupWindowPtr);
        }
        profiler.popFrame();
        swapBuffers();
    }

    @Override
    public void pollEvents() {
        profiler.pushFrame("poll-events");
        super.pollEvents();
        profiler.popFrame();
    }
}
