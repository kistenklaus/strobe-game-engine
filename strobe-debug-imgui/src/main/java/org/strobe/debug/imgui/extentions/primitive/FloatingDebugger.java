package org.strobe.debug.imgui.extentions.primitive;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Options;
import org.joml.Runtime;
import org.joml.Vector2i;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

public class FloatingDebugger extends TypeDebugger {

    private final float fontSize;

    public FloatingDebugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "0123456790.e");
        fontSize = temp.y;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(150, 50);
    }

    @Override
    public Class<?>[] getTypesToApply() {
        return new Class[]{float.class, Float.class, double.class, Double.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        String typeName;
        String valueStr;
        if (value.getClass().equals(Float.class)) {
            typeName = "float";
            valueStr = NumberFormatUtil.formatDouble((Float) value);
        } else if (value.getClass().equals(Double.class)) {
            typeName = "double";
            valueStr = NumberFormatUtil.formatDouble((Double) value);
        } else {
            throw new IllegalArgumentException("FloatingDebugger can only debug floating point numbers!");
        }
        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);
        ImGui.text(typeName + ":");
        ImGui.sameLine();
        ImGui.button(valueStr, 75, fontSize * 1.25f);
    }

    private String formatDouble(double value) {
        String normal = Double.toString(value);
        if (normal.length() < 8) return normal;
        return Runtime.format(value, Options.NUMBER_FORMAT);
    }
}
