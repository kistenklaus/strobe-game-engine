package org.strobe.debug.imgui.extentions.opengl;

import imgui.ImGui;
import org.joml.*;
import org.strobe.debug.NumberFormatUtil;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;

import java.lang.reflect.Type;
import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL15.GL_READ_ONLY;

public class UboDebugger extends TypeDebugger {

    private static final int BUTTON_HEIGHT = 20;
    private static final int BUTTON_WIDTH = 80;
    private static final int BUTTON_PADDING_HOR = 5;
    private static final int BUTTON_PADDING_VER = 3;
    private static final int LOCATION_PADDING = 20;
    private static final int LOCATION_TITLE_PADDING = 3;
    private static final int PADDING_HOR = 10;
    private static final int PADDING_VER = 15;

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        Ubo ubo = (Ubo) object;
        Type[] types = ubo.getTypes();
        int height = BUTTON_PADDING_VER;
        for (Type type : types) {
            if (type == Matrix4f.class) {
                height += 4 * (BUTTON_HEIGHT + BUTTON_PADDING_VER);
            } else if (type == Matrix3f.class) {
                height += 3 * (BUTTON_HEIGHT + BUTTON_PADDING_VER);
            } else if (type == Matrix2f.class) {
                height += 2* (BUTTON_HEIGHT + BUTTON_PADDING_VER);
            } else {
                height += BUTTON_HEIGHT + BUTTON_PADDING_VER;
            }
            height += LOCATION_PADDING + LOCATION_TITLE_PADDING;
        }
        height += PADDING_VER * 2;

        return new Vector2i(4 * BUTTON_WIDTH * 2 * BUTTON_PADDING_HOR + 2 * PADDING_HOR, height);
    }

    @Override
    protected Type[] getTypesToApply() {
        return new Type[]{Ubo.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        Ubo ubo = (Ubo) value;
        Type[] types = ubo.getTypes();
        Type[] baseTypes = new Type[types.length];
        int[] baseTypeLengths = new int[types.length];
        int[] buttonsPerLines = new int[types.length];
        int[] buttonCount = new int[types.length];
        for (int i = 0; i < types.length; i++) {
            Type t = types[i];
            if (t == int.class || t == Vector2i.class || t == Vector3i.class || t == Vector4i.class) {
                baseTypes[i] = int.class;
                baseTypeLengths[i] = Integer.BYTES;
            } else {
                baseTypes[i] = float.class;
                baseTypeLengths[i] = Float.BYTES;
            }

            System.out.println(t);
            if(t == Vector3f.class || t == Vector2i.class){
                buttonCount[i] = 3;
            }else if(t == Vector4f.class || t == Vector4i.class){
                buttonCount[i] = 4;
            }else if(t == Vector2f.class || t == Vector2i.class){
                buttonCount[i] = 2;
            }else if(t == Matrix4f.class) buttonCount[i] = 16;
            else if(t == Matrix3f.class) buttonCount[i] = 16;
            else if(t == Matrix2f.class) buttonCount[i] = 4;
            else if(t == Float.class)buttonCount[i] = 1;
            else throw new IllegalStateException();


            if (t == Matrix4f.class) buttonsPerLines[i] = 4;
            else if (t == Matrix3f.class) buttonsPerLines[i] = 3;
            else if (t == Matrix2f.class) buttonsPerLines[i] = 2;
            else buttonsPerLines[i] = Integer.MAX_VALUE;
        }


        ByteBuffer buffer = ubo.mapBuffer(gfx, GL_READ_ONLY);

        float cursorX = PADDING_HOR;
        float cursorY = PADDING_VER*1.5f;
        ImGui.setCursorPos(cursorX, cursorY);

        for (int i = 0; i < types.length; i++) {
            cursorY += LOCATION_PADDING;
            //drawTitle:
            ImGui.text(types[i].toString());
            ImGui.setCursorPos(cursorX, cursorY);
            cursorY += LOCATION_TITLE_PADDING;

            buffer.position(ubo.getOffsets()[i]);
            int lineCounter = 0;
            for (int j = 0; j < buttonCount[i]; j++, lineCounter++) {
                if(lineCounter >= buttonsPerLines[i]){
                    //new Line
                    cursorX -= lineCounter*(BUTTON_WIDTH + BUTTON_PADDING_HOR);
                    cursorY += BUTTON_HEIGHT + BUTTON_PADDING_VER;
                    lineCounter -= buttonsPerLines[i];
                }
                ImGui.setCursorPos(cursorX, cursorY);

                String strValue;
                if (baseTypes[i] == int.class) {
                    strValue = NumberFormatUtil.formatLong(buffer.getInt());
                } else if (baseTypes[i] == float.class) {
                    strValue = NumberFormatUtil.formatDouble(buffer.getFloat());
                } else throw new IllegalStateException();

                ImGui.button(strValue, BUTTON_WIDTH, BUTTON_HEIGHT);

                cursorX += BUTTON_WIDTH + BUTTON_PADDING_HOR;
            }
            cursorX -= lineCounter * (BUTTON_WIDTH + BUTTON_PADDING_HOR);
            cursorY += BUTTON_HEIGHT;
        }

        buffer.flip();
        ubo.unmapBuffer(gfx);

    }
}
