package org.strobe.debug.imgui;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector2i;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;


public abstract class TypeDebugger {

    private static final int TITLE_PADDING_HOR = 15;

    protected float typeBoxWidth;
    protected float typeBoxHeight;
    private final Type[] types;
    protected boolean newLineDebugger = false;
    protected ImGuiDebugger renderer;

    protected float windowScrollY;
    protected float windowScrollX;
    protected float windowWidth;
    protected float windowHeight;
    protected String fieldName;

    public TypeDebugger() {
        this.types = getTypesToApply();
    }

    void link(ImGuiDebugger renderer) {
        this.renderer = renderer;
    }

    public abstract Vector2i calculateBoxDimension(Object object);

    public void configureFor(Object object, String name) {
        Vector2i boxDim = calculateBoxDimension(object);
        ImVec2 texSize = new ImVec2();
        ImGui.calcTextSize(texSize,name);
        typeBoxWidth = Math.max(boxDim.x, texSize.x + TITLE_PADDING_HOR*2);
        typeBoxHeight = boxDim.y;
        this.fieldName = name;
    }

    public void setWindowScroll(float x, float y) {
        windowScrollX = x;
        windowScrollY = y;
    }


    public void setWindowSize(float x, float y) {
        windowWidth = x;
        windowHeight = y;
    }

    protected abstract Type[] getTypesToApply();

    public Type[] getTypes(){
        return types;
    }

    public abstract void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY);

    public float getBoxWidth() {
        return typeBoxWidth;
    }

    public float getBoxHeight() {
        return typeBoxHeight;
    }
}
