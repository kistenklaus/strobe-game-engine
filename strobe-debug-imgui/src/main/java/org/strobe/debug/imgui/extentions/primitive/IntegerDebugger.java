package org.strobe.debug.imgui.extentions.primitive;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector2i;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;

public class IntegerDebugger extends TypeDebugger {

    private final float fontSize;

    public IntegerDebugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "0123456790.e");
        fontSize = temp.y;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(150, 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Integer.class, int.class, Long.class, long.class, Short.class, short.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        String typeName;
        String valueStr;
        if (value.getClass().equals(Float.class)) {
            typeName = "short";
            valueStr = NumberFormatUtil.formatLong((short) value);
        } else if (value.getClass().equals(Integer.class)) {
            typeName = "int";
            System.out.println(value.getClass());
            valueStr = NumberFormatUtil.formatLong((int)value);
        } else if (value.getClass().equals(Long.class)) {
            typeName = "long";
            valueStr = NumberFormatUtil.formatLong((long) value);
        } else throw new IllegalArgumentException("IntegerDebugger can only debug floating point numbers!");

        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);
        ImGui.text(typeName + ":");
        ImGui.sameLine();
        ImGui.button(valueStr, 75, fontSize * 1.25f);
    }
}
