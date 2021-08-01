package org.strobe.debug.imgui.extentions.joml;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiStyleVar;
import org.joml.*;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;

public class Vector3Debugger extends TypeDebugger {

    private static final float COMPONENT_VALUE_WIDTH = 75;

    private final float fontSize;

    public Vector3Debugger() {
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
        return new Type[]{Vector3f.class, Vector3fc.class, Vector3i.class, Vector3ic.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        String formatX;
        String formatY;
        String formatZ;
        if (value instanceof Vector3fc) {
            Vector3fc vec3f = (Vector3fc) value;
            formatX = NumberFormatUtil.formatDouble(vec3f.x());
            formatY = NumberFormatUtil.formatDouble(vec3f.y());
            formatZ = NumberFormatUtil.formatDouble(vec3f.z());
        } else if (value instanceof Vector3ic) {
            Vector3ic vec3i = (Vector3ic) value;
            formatX = NumberFormatUtil.formatLong(vec3i.x());
            formatY = NumberFormatUtil.formatLong(vec3i.y());
            formatZ = NumberFormatUtil.formatLong(vec3i.z());
        } else throw new IllegalArgumentException("Vector3Debugger doesn't accept this Class!");

        ImGui.pushStyleVar(ImGuiStyleVar.ButtonTextAlign, 0.5f, 0f);

        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);
        ImGui.text("x:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatX, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);
        ImGui.sameLine();
        ImGui.text("y:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatY, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);
        ImGui.sameLine();
        ImGui.text("z:");
        ImGui.sameLine();
        ImGui.setCursorPosX(ImGui.getCursorPosX() - 5f);
        ImGui.button(formatZ, COMPONENT_VALUE_WIDTH, fontSize * 1.25f);

        ImGui.popStyleVar();
    }
}
