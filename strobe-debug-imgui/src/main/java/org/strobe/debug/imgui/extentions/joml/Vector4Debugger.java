package org.strobe.debug.imgui.extentions.joml;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiStyleVar;
import org.joml.*;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;

public class Vector4Debugger extends TypeDebugger {

    private static final float COMPONENT_VALUE_WIDTH = 75;
    private static final float VECTOR_PADDING = 0;

    private final float fontSize;

    public Vector4Debugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "0123456790.e");
        fontSize = temp.y;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(470, 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Vector4f.class, Vector4fc.class, Vector4i.class, Vector4ic.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        String formatX;
        String formatY;
        String formatZ;
        String formatW;
        if (value instanceof Vector4fc) {
            Vector4fc vec4f = (Vector4fc) value;
            formatX = NumberFormatUtil.formatDouble(vec4f.x());
            formatY = NumberFormatUtil.formatDouble(vec4f.y());
            formatZ = NumberFormatUtil.formatDouble(vec4f.z());
            formatW = NumberFormatUtil.formatDouble(vec4f.w());
        } else if (value instanceof Vector4ic) {
            Vector4ic vec4i = (Vector4ic) value;
            formatX = NumberFormatUtil.formatLong(vec4i.x());
            formatY = NumberFormatUtil.formatLong(vec4i.y());
            formatZ = NumberFormatUtil.formatLong(vec4i.z());
            formatW = NumberFormatUtil.formatLong(vec4i.w());
        } else throw new IllegalArgumentException("Vector4Debugger doesn't accept this Class!");

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
        ImGui.sameLine();
        ImGui.dummy(VECTOR_PADDING, 0);
        ImGui.sameLine();
        ImGui.text("z:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatZ, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);
        ImGui.sameLine();
        ImGui.dummy(VECTOR_PADDING, 0);
        ImGui.sameLine();
        ImGui.text("w:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatW, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);

        ImGui.popStyleVar();

    }
}
