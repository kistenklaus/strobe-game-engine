package org.strobe.debug.imgui.extentions.primitive;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector2i;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;


public class CharacterDebugger extends TypeDebugger {

    private final float fontSize;

    public CharacterDebugger() {
        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "01");
        fontSize = temp.y;
    }


    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i(150, 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Character.class, char.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        ImGui.setCursorPos(10, getBoxHeight() / 2f - fontSize / 2f);
        ImGui.text("char:");
        ImGui.sameLine();
        ImGui.button("\'" + value + "\'", 75, fontSize * 1.25f);
    }
}
