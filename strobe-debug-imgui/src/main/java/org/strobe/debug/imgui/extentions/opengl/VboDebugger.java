package org.strobe.debug.imgui.extentions.opengl;

import imgui.ImGui;
import org.joml.Vector2i;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.vao.Vbo;

import java.lang.reflect.Type;

public class VboDebugger extends TypeDebugger {

    private static final boolean ENABLE = false;

    private static final int BUTTON_HEIGHT = 20;
    private static final int BUTTON_WIDTH = 60;
    private static final int BUTTON_PADDING_HOR = 5;
    private static final int BUTTON_PADDING_VER = 2;
    private static final int PADDING_HOR = 10;
    private static final int PADDING_TOP = 20;
    private static final int PADDING_BOTTOM = 10;

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        if(!ENABLE)return new Vector2i(0);
        Vbo vbo = (Vbo) object;
        int vertexCount = vbo.getCapacity() / vbo.getVertexSize();
        int vertexSize = vbo.getVertexSize() / Float.BYTES;

        return new Vector2i(
                vertexSize * BUTTON_WIDTH + BUTTON_PADDING_HOR * (vertexSize - 1)
                        + 2 * PADDING_HOR,
                BUTTON_HEIGHT * vertexCount + BUTTON_PADDING_VER * (vertexSize - 1)
                        + PADDING_BOTTOM + PADDING_TOP);
    }

    @Override
    protected Type[] getTypesToApply() {
        return new Type[]{Vbo.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        if(!ENABLE)return;
        Vbo vbo = (Vbo) value;
        int vertexCount = vbo.getCapacity() / vbo.getVertexSize();
        int vertexSize = vbo.getVertexSize() / Float.BYTES;
        float[] content = vbo.getBufferAsFloatArray(gfx);
        float cursorX = PADDING_HOR;
        float cursorY = PADDING_TOP;
        ImGui.setCursorPos(cursorX, cursorY);
        for (int i = 0; i < vertexCount; i++) {
            for (int j = 0; j < vertexSize; j++) {
                ImGui.setCursorPos(cursorX, cursorY);
                ImGui.button("" + content[i * vertexSize + j], BUTTON_WIDTH, BUTTON_HEIGHT);
                cursorX += BUTTON_WIDTH + BUTTON_PADDING_HOR;
            }
            cursorX -= BUTTON_WIDTH * vertexSize + BUTTON_PADDING_HOR * vertexSize;
            cursorY += BUTTON_HEIGHT + BUTTON_PADDING_VER;
        }
    }
}
