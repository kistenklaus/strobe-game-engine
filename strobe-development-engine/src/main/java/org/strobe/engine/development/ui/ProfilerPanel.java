package org.strobe.engine.development.ui;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiTreeNodeFlags;
import imgui.type.ImBoolean;
import org.strobe.engine.development.profiler.Profiler;
import org.strobe.engine.development.profiler.ProfilerCompleteFrame;
import org.strobe.gfx.Graphics;

public final class ProfilerPanel implements UIPanel {

    private final DevelopmentStyle style;
    private final Profiler profiler;
    private static final int LAYER_SIZE = 20;
    private ImBoolean calcAvg;

    public ProfilerPanel(DevelopmentStyle style, Profiler profiler) {
        this.style = style;
        this.profiler = profiler;
    }

    @Override
    public void init(Graphics gfx) {
        calcAvg = new ImBoolean(false);
    }

    @Override
    public void draw(Graphics gfx) {
        if (ImGui.begin("Profiler")) {
            ImGui.pushID("Profiler");

            ImGui.bulletText("frame-time : " + String.format("%.2f", profiler.getTrueFrameTime() * 1e3f) + "ms/" + 1.0f / profiler.getTrueFrameTime() + "FPS");

            boolean enableHistory = false;
            boolean enableAverage = false;

            if (ImGui.treeNodeEx("frame time history", ImGuiTreeNodeFlags.Bullet | ImGuiTreeNodeFlags.SpanFullWidth)){
                enableHistory = true;
                float min = Float.MAX_VALUE;
                float max = Float.MIN_VALUE;
                float[] durations = new float[profiler.getHistorySize()];
                int i = 0;
                for (ProfilerCompleteFrame frame : profiler.getHistory()) {
                    if (frame.getDuration() > max) max = frame.getDuration();
                    if (frame.getDuration() < min) min = frame.getDuration();
                    durations[i++] = frame.getDuration();
                }
                ImGui.plotLines("frame-times", durations, Profiler.MAX_HISTORY_LENGTH, 0, "", min, max,
                        ImGui.getContentRegionAvailX(), LAYER_SIZE * 2);
                ImGui.treePop();
            }


            if (ImGui.treeNodeEx("flame graph", ImGuiTreeNodeFlags.Bullet | ImGuiTreeNodeFlags.SpanFullWidth)) {
                enableAverage = true;
                ProfilerCompleteFrame frame = profiler.getAvgFrame();


                if (ImGui.isItemHovered()) {
                    float progress = profiler.getHistorySize() / (float) Profiler.MAX_HISTORY_LENGTH;
                    if (progress < 1.0f) {
                        ImGui.setTooltip("collecting frame history..." + String.format("%.2f", progress * 100) + "%\r");
                    }
                }

                if (frame != null) {
                    ImVec2 region = ImGui.getContentRegionAvail();
                    ImVec2 cp = ImGui.getCursorPos();
                    drawProfilingGraph(cp, 0, region.x, frame, 0);

                }

                ImGui.treePop();
            }

            if (enableAverage) profiler.enableAvgFrame();
            else if (enableHistory) profiler.enableHistory();
            else profiler.disableHistory();

            ImGui.popID();
        } else {
            profiler.disablePrevFrame();
        }
        ImGui.end();
    }

    private void drawProfilingGraph(ImVec2 o, float start, float end, ProfilerCompleteFrame frame, int depth) {

        float width = end - start;

        float sp = start + width * frame.getStart();
        float dp = width * (frame.getEnd() - frame.getStart());

        ImGui.setCursorPos(o.x + sp, o.y + LAYER_SIZE * depth);

        ImGui.button(frame.getName(), dp, LAYER_SIZE);
        if (ImGui.isItemHovered()) {
            ImGui.setTooltip(frame.getName() + " took " + String.format("%.2f", frame.getDuration()) + "ms");
        }

        for (ProfilerCompleteFrame child : frame.children()) {
            drawProfilingGraph(o, sp, sp + dp, child, depth + 1);
        }

    }
}
