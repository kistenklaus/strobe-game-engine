package org.strobe.debug.imgui.extentions.opengl;

import imgui.ImGui;
import org.joml.Vector2i;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

import java.lang.reflect.Type;

public class Texture2DDebugger extends TypeDebugger {

    private static final int COL_ONE_OFFSET = 20;
    private static final int COL_TWO_OFFSET = 120;

    private static final int TEXTURE_HEIGHT = 200;

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        return new Vector2i((int) (TEXTURE_HEIGHT*1.5 + 300), TEXTURE_HEIGHT + 50);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Texture2D.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        if (value.getClass().equals(Texture2D.class)) {
            Texture2D tex2D = (Texture2D) value;
            int w = tex2D.getWidth();
            int h = tex2D.getHeight();
            int nh = 200;
            int nw = (int) Math.floor((w / (float) h) * nh);
            float midY = getBoxHeight() / 2.0f;
            ImGui.setCursorPos(10, 15+(getBoxHeight()-15) / 2.0f - nh / 2.0f);
            ImGui.image(tex2D.getID(), nw, nh);

            ImGui.setCursorPos(nw+COL_ONE_OFFSET, midY - ImGui.getFontSize());
            ImGui.text("width: " + tex2D.getWidth());

            ImGui.setCursorPos(nw+COL_TWO_OFFSET, midY - ImGui.getFontSize());
            ImGui.text("height: " + tex2D.getHeight());
            ImGui.setCursorPos(nw+COL_ONE_OFFSET, midY);
            ImGui.text("channels: " + tex2D.getFormat().getChannels());
            ImGui.setCursorPos(nw+COL_TWO_OFFSET, midY);
            ImGui.text("samples: " + tex2D.getSamples());
            ImGui.setCursorPos(nw+COL_ONE_OFFSET, midY+ImGui.getFontSize());
            ImGui.text("format: " + tex2D.getFormat().name());
        }
    }
}
