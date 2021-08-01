package org.strobe.debug.imgui.extentions;


import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector2i;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;

public class ObjectDebugger extends TypeDebugger {

    private final float fontSize;

    public ObjectDebugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "ABCEFGHIJKLMNOPQRSRTUVWXYT.0123456789@#&$");
        fontSize = temp.y;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(310, 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Object.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);
        ImGui.text(value.getClass().getSimpleName() + ":");
        ImGui.sameLine();
        float curX = ImGui.getCursorPosX();
        float remaining = getBoxWidth() - 10 - curX;
        ImGui.button(value.toString(), remaining, fontSize * 1.25f);
    }
}
