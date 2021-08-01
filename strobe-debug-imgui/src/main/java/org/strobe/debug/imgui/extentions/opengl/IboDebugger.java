package org.strobe.debug.imgui.extentions.opengl;

import imgui.ImGui;
import org.joml.Vector2i;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.vao.Ibo;

import java.lang.reflect.Type;

public class IboDebugger extends TypeDebugger {

    private static final boolean ENABLE = false;

    private static final int BUTTON_HEIGHT = 20;
    private static final int BUTTON_WIDTH = 60;
    private static final int BUTTON_PADDING_HOR = 5;
    private static final int BUTTON_PADDING_VER = 3;
    private static final int PADDING_HOR = 10;
    private static final int PADDING_TOP = 20;
    private static final int PADDING_BOTTOM = 10;

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        if(!ENABLE)return new Vector2i(0,0);
        Ibo ibo = (Ibo) object;
        int size = ibo.getCapacity() / Integer.BYTES;
        int height = (int) Math.ceil(size / 3f) * (BUTTON_HEIGHT + BUTTON_PADDING_VER);
        height -= BUTTON_PADDING_VER;
        height += PADDING_TOP + PADDING_BOTTOM;
        return new Vector2i(BUTTON_WIDTH * 3 + BUTTON_PADDING_HOR * 2 + 2 * PADDING_HOR, height);
    }

    @Override
    protected Type[] getTypesToApply() {
        return new Type[]{Ibo.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        if(!ENABLE)return;
        Ibo ibo = (Ibo) value;
        int[] data = ibo.getBufferAsIntegerArray(gfx);


        float cursorX = PADDING_HOR;
        float cursorY = PADDING_TOP;

        for (int i = 0, j = 0; i < data.length; i++, j++) {
            if (j >= 3) {
                cursorX -= j * (BUTTON_WIDTH + BUTTON_PADDING_HOR);
                cursorY += BUTTON_HEIGHT + BUTTON_PADDING_VER;
                j-=3;
            }
            ImGui.setCursorPos(cursorX, cursorY);
            ImGui.button(NumberFormatUtil.formatLong(data[i]), BUTTON_WIDTH, BUTTON_HEIGHT);
            cursorX += BUTTON_WIDTH + BUTTON_PADDING_HOR;
        }


    }
}
