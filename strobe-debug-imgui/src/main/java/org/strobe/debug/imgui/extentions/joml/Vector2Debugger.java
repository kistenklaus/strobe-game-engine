package org.strobe.debug.imgui.extentions.joml;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiStyleVar;
import org.joml.Runtime;
import org.joml.*;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;

public class Vector2Debugger extends TypeDebugger {

    private static final float COMPONENT_VALUE_WIDTH = 75;
    private static final float VECTOR_PADDING = 20;

    private final float fontSize;

    public Vector2Debugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "0123456790.e");
        fontSize = temp.y;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(310, 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Vector2f.class, Vector2fc.class, Vector2i.class, Vector2fc.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        String formatX;
        String formatY;
        if (value instanceof Vector2fc) {
            Vector2fc vec3f = (Vector2fc) value;
            formatX = NumberFormatUtil.formatDouble(vec3f.x());
            formatY = NumberFormatUtil.formatDouble(vec3f.y());
        } else if (value instanceof Vector2ic) {
            Vector2ic vec3i = (Vector2ic) value;
            formatX = NumberFormatUtil.formatLong(vec3i.x());
            formatY = NumberFormatUtil.formatLong(vec3i.y());
        } else throw new IllegalArgumentException("Vector2Debugger doesn't accept this Class!");

        ImGui.pushStyleVar(ImGuiStyleVar.ButtonTextAlign, 0.5f, 0f);

        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);

        ImGui.dummy(VECTOR_PADDING, 0);
        ImGui.sameLine();
        ImGui.text("x:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatX, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);
        ImGui.sameLine();
        ImGui.dummy(VECTOR_PADDING, 0);
        ImGui.sameLine();
        ImGui.text("y:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatY, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);

        ImGui.popStyleVar();
    }

    private String formatDouble(double value) {
        String normal = Double.toString(value);
        if (normal.length() < 8) return normal;
        return Runtime.format(value, Options.NUMBER_FORMAT);
    }

    private String formatLong(long value) {
        String normal = Long.toString(value);
        //scientific notation
        return normal;
    }
}
