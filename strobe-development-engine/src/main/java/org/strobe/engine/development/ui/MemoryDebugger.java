package org.strobe.engine.development.ui;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiMouseButton;
import imgui.flag.ImGuiTableFlags;
import imgui.flag.ImGuiTreeNodeFlags;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.framebuffer.FramebufferAttachment;
import org.strobe.gfx.opengl.bindables.framebuffer.FramebufferPool;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TexturePool2D;
import org.strobe.gfx.opengl.bindables.util.TypeUtil;
import org.strobe.gfx.opengl.bindables.vao.Vbo;
import org.strobe.gfx.opengl.bindables.vao.VboPool;
import org.strobe.window.imgui.ImGuiUtils;

import java.awt.*;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

public final class MemoryDebugger implements UIPanel {


    private static final int TEXTURE_SIZE = 100;
    private static final int TEXTURE_POPUP_SIZE = 150;
    private static final int TEXTURE_PADDING = 10;
    private static final float TEXTURE_ZOOM = 2f;


    private final Map<Integer, Integer> selectedAttachment = new TreeMap<>();

    @Override
    public void init(Graphics gfx) {

    }

    @Override
    public void draw(Graphics gfx) {
        if (ImGui.begin("MemoryDebugger")) {

            if (ImGui.treeNodeEx("texture", ImGuiTreeNodeFlags.SpanFullWidth)) {
                ImGui.pushID("textures");
                TexturePool2D texPool = gfx.getPool(TexturePool2D.class);
                float region = Math.max(TEXTURE_SIZE * 2, ImGui.getContentRegionAvailX());
                ImVec2 cp = ImGui.getCursorPos();

                float lineHeight = 0;
                float xOffset = cp.x;
                float yOffset = cp.y;

                for (Texture2D tex : texPool.textures()) {

                    final float aspect = tex.getWidth() / (float) tex.getHeight();
                    final float w = TEXTURE_SIZE * aspect;
                    final float h = TEXTURE_SIZE;

                    if (xOffset + w > region) {
                        xOffset = cp.x;
                        yOffset += lineHeight + TEXTURE_PADDING;
                        lineHeight = 0;
                    }

                    ImGui.setCursorPos(xOffset, yOffset);

                    ImVec2 scp = ImGui.getCursorScreenPos();
                    ImGuiUtils.image(tex.getID(), w, h, Color.BLACK);
                    if (ImGui.isItemHovered()) {
                        ImGui.beginTooltip();
                        if (ImGui.isMouseDown(ImGuiMouseButton.Left)) {
                            ImVec2 mp = ImGui.getMousePos();
                            mp.x -= scp.x;
                            mp.y -= scp.y;
                            mp.x /= w;
                            mp.y /= h;

                            float zs = 0.5f / TEXTURE_ZOOM;
                            if (mp.x > 1 - zs) mp.x = 1 - zs;
                            else if (mp.x < zs) mp.x = zs;
                            if (mp.y > 1 - zs) mp.y = 1 - zs;
                            else if (mp.y < zs) mp.y = zs;

                            float uv0x = mp.x - zs;
                            float uv0y = 1 - (mp.y - zs);
                            float uv1x = mp.x + zs;
                            float uv1y = 1 - (mp.y + zs);
                            ImGui.image(tex.getID(), TEXTURE_POPUP_SIZE * aspect, TEXTURE_POPUP_SIZE, uv0x, uv0y, uv1x, uv1y);
                        } else {
                            ImGui.image(tex.getID(), TEXTURE_POPUP_SIZE * aspect, TEXTURE_POPUP_SIZE, 0, 1, 1, 0);
                        }

                        ImGui.sameLine();

                        ImVec2 rcp = ImGui.getCursorPos();
                        ImGui.text("width  :" + tex.getWidth());
                        ImGui.setCursorPos(rcp.x, rcp.y + ImGui.getFontSize());
                        ImGui.text("height :" + tex.getHeight());

                        ImGui.setCursorPos(rcp.x, rcp.y + 2 * ImGui.getFontSize());
                        ImGui.text("format :" + tex.getFormat().name());

                        ImGui.setCursorPos(rcp.x, rcp.y + 3 * ImGui.getFontSize());
                        ImGui.text("samples:" + tex.getSamples());

                        ImGui.endTooltip();
                    }
                    if (xOffset + w <= region) {
                        xOffset += w + TEXTURE_PADDING;
                        lineHeight = Math.max(lineHeight, h);
                    }
                }

                ImGui.popID();
                ImGui.treePop();
            }
            if (ImGui.treeNodeEx("framebuffers", ImGuiTreeNodeFlags.SpanFullWidth)) {
                ImGui.pushID("framebuffers");

                FramebufferPool fboPool = gfx.getPool(FramebufferPool.class);

                for (Framebuffer fbo : fboPool) {
                    if (ImGui.treeNodeEx("id=" + fbo.getID(), ImGuiTreeNodeFlags.SpanFullWidth)) {

                        float tx = ImGui.getCursorPosX();
                        if(ImGui.beginTable("dimensions", 3, ImGuiTableFlags.RowBg| ImGuiTableFlags.Borders
                                | ImGuiTableFlags.SizingFixedSame | ImGuiTableFlags.NoHostExtendX)){

                            ImGui.tableSetupColumn("width");
                            ImGui.tableSetupColumn("height");
                            ImGui.tableSetupColumn("attachments");
                            ImGui.tableHeadersRow();

                            ImGui.tableNextRow();
                            ImGui.tableSetColumnIndex(0);
                            ImGui.text(fbo.getWidth()+"px");

                            ImGui.tableSetColumnIndex(1);
                            ImGui.text(fbo.getHeight()+"px");

                            ImGui.tableSetColumnIndex(2);
                            ImGui.text(fbo.getAttachmentCount()+"");

                            ImGui.endTable();
                        }
                        ImGui.sameLine();
                        float tableWidth = ImGui.getCursorPosX()-tx - 8;
                        ImGui.newLine();

                        Integer selectedIndex = selectedAttachment.computeIfAbsent(fbo.getID(), k -> 0);

                        Iterator<Framebuffer.Attachment> attIt = fbo.getAttachmentIterable().iterator();
                        for(int i=0;i<selectedIndex;i++) attIt.next();
                        Framebuffer.Attachment selected = attIt.next();

                        int i = 0;
                        ImGui.pushID("attachments");
                        ImGui.pushItemWidth(tableWidth);
                        if (ImGui.beginCombo("", selected.name())) {
                            for (Framebuffer.Attachment attachment : fbo.getAttachmentIterable()) {
                                if (ImGui.selectable(attachment.name(), selectedIndex == i)) {
                                    selectedIndex = i;
                                    selectedAttachment.put(fbo.getID(), selectedIndex);
                                }
                                i++;
                            }
                            ImGui.endCombo();
                        }
                        ImGui.popID();


                        ImVec2 cp = ImGui.getCursorPos();

                        FramebufferAttachment attachment = fbo.getAttachment(selected);
                        ImGui.checkbox("has Depth", attachment.hasDepth());
                        ImGui.checkbox("has Stencil", attachment.hasStencil());
                        ImGui.checkbox("is Textured", attachment.isTextured());


                        if(attachment.isTextured()){
                            ImVec2 cp2 = ImGui.getCursorPos();
                            ImGui.setCursorPos(cp.x + 150, cp.y);
                            float aspect = fbo.getWidth()/(float)fbo.getHeight();
                            float height = cp2.y - cp.y;
                            ImGuiUtils.image(attachment.getTexture().getID(), aspect*height, height, Color.BLACK);
                        }

                        ImGui.treePop();
                    }
                }

                ImGui.popID();
                ImGui.treePop();
            }

            if (ImGui.treeNodeEx("vertex-buffers")) {
                ImGui.pushID("vertex-buffers");
                VboPool vboPool = gfx.getPool(VboPool.class);

                for (Vbo vbo : vboPool.buffers()) {
                    if (ImGui.treeNodeEx("id=" + vbo.getID(), ImGuiTreeNodeFlags.SpanFullWidth)) {
                        if (ImGui.beginTable("vertex attributes", 5, ImGuiTableFlags.Borders | ImGuiTableFlags.RowBg | ImGuiTableFlags.SizingFixedSame | ImGuiTableFlags.Resizable
                                | ImGuiTableFlags.NoHostExtendX)) {

                            ImGui.tableSetupColumn("location");
                            ImGui.tableSetupColumn("type");
                            ImGui.tableSetupColumn("offset");
                            ImGui.tableSetupColumn("stride");
                            ImGui.tableSetupColumn("coverage");
                            ImGui.tableHeadersRow();

                            int attribCount = vbo.getLocations().length;

                            for (int i = 0; i < attribCount; i++) {
                                ImGui.tableNextRow();

                                ImGui.tableSetColumnIndex(0);
                                ImGui.text("" + vbo.getLocations()[i]);

                                ImGui.tableSetColumnIndex(1);
                                ImGui.text(TypeUtil.getNameOfGlType(vbo.getTypes()[i]));

                                ImGui.tableSetColumnIndex(2);
                                ImGui.text("" + vbo.getOffsets()[i]);

                                ImGui.tableSetColumnIndex(3);
                                ImGui.text("" + vbo.getStrides()[i]);

                                ImGui.tableSetColumnIndex(4);
                                ImGui.text("" + vbo.getCoverages()[i]);
                            }
                            ImGui.endTable();
                        }
                        ImGui.treePop();
                    }
                }
                ImGui.popID();
                ImGui.treePop();
            }
        }
        ImGui.end();
    }
}
