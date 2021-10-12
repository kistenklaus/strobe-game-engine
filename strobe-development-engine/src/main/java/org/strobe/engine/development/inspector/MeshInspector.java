package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiTreeNodeFlags;
import imgui.type.ImBoolean;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.mesh.Mesh;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

import java.util.function.Consumer;

public final class MeshInspector extends ComponentInspector<Mesh> {

    private boolean queryVertexData = false;
    private final float[] v3 = new float[3];
    private final float[] v2 = new float[2];
    private final int[] f3 = new int[3];

    @Override
    public Class<? extends Mesh>[] getInspectorTargets() {
        return new Class[]{Mesh.class};
    }

    @Override
    protected void drawInspectorNode(Graphics gfx, DevelopmentStyle style, Mesh mesh, Entity entity) {

        ImVec2 cp = ImGui.getCursorPos();

        ImGui.setCursorPos(cp.x, cp.y + 1);
        ImGui.bulletText("query vertex data");

        ImBoolean query = new ImBoolean(queryVertexData);
        ImGui.pushID("query");
        ImGui.setCursorPos(cp.x + 200, cp.y);
        if (ImGui.checkbox("", query)) {
            queryVertexData = query.get();
        }
        ImGui.popID();

        float[] positions;
        float[] uvs;
        float[] normals;
        float[] tangents;
        float[] bitangents;
        int vertexCount = mesh.getVertexCount();

        int[] indices;
        int drawCount = mesh.getDrawCount();
        int indicesCount = mesh.getIndicesCount();
        if (queryVertexData) {
            positions = mesh.nativeFetchPosition(gfx);
            uvs = mesh.nativeFetchTextureCoords(gfx);
            normals = mesh.nativeFetchNormals(gfx);
            tangents = mesh.nativeFetchTangents(gfx);
            bitangents = mesh.nativeFetchBitangents(gfx);
            indices = mesh.nativeFetchIndices(gfx);
        } else {
            positions = mesh.getPositions();
            uvs = mesh.getTextureCoords();
            normals = mesh.getNormals();
            tangents = mesh.getTangents();
            bitangents = mesh.getBitangents();
            indices = mesh.getIndices();
        }


        int flags = ImGuiTreeNodeFlags.Bullet | ImGuiTreeNodeFlags.SpanAvailWidth;

        final float PADDING = 5;

        if (indices != null) {
            ImGui.pushID("indices");
            if (ImGui.treeNodeEx("indices (faces)", flags)) {
                indexData(indices, indicesCount / 3, drawCount / 3, i -> mesh.setIndices(0, indices));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);
            ImGui.popID();
        }

        if (positions != null) {
            ImGui.pushID("positions");
            if (ImGui.treeNodeEx("positions", flags)) {
                vertexDataVec3(positions, vertexCount, i -> mesh.setPositions(0, positions));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);

            ImGui.popID();
        }

        if (uvs != null) {
            ImGui.pushID("uvs");
            if (ImGui.treeNodeEx("textureCoords", flags)) {
                vertexDataVec2(uvs, vertexCount, i -> mesh.setTextureCoords(0, uvs));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);
            ImGui.popID();
        }

        if (normals != null) {
            ImGui.pushID("normals");
            if (ImGui.treeNodeEx("normals", flags)) {
                vertexDataVec3(normals, vertexCount, i -> mesh.setNormals(0, normals));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);
            ImGui.popID();
        }

        if (tangents != null) {
            ImGui.pushID("tangents");
            if (ImGui.treeNodeEx("tangents", flags)) {
                vertexDataVec3(tangents, vertexCount, i -> mesh.setTangents(0, tangents));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);
            ImGui.popID();
        }

        if (bitangents != null) {
            ImGui.pushID("bitangents");
            if (ImGui.treeNodeEx("bitangents", flags)) {
                vertexDataVec3(bitangents, vertexCount, i -> mesh.setBitangents(0, bitangents));
                ImGui.treePop();
            }
            ImGui.setCursorPosY(ImGui.getCursorPosY() + PADDING);
            ImGui.popID();
        }
    }

    private void vertexDataVec3(float[] data, int vertexCount, Consumer<Integer> updateFunc) {
        ImVec2 rcp = ImGui.getCursorPos();
        boolean change = false;
        int firstChange = 0;
        for (int i = 0; i < vertexCount; i++) {
            v3[0] = data[i * 3];
            v3[1] = data[i * 3 + 1];
            v3[2] = data[i * 3 + 2];
            drawIndex(rcp, i);
            ImGui.pushID(i);
            ImGui.pushItemWidth(250);
            ImGui.setCursorPos(rcp.x + 50, rcp.y + 25 * i);
            boolean c = ImGui.dragFloat3("", v3, 0.01f);
            if (c) {
                if (!change) firstChange = i;
                change = true;
                data[i * 3] = v3[0];
                data[i * 3 + 1] = v3[1];
                data[i * 3 + 2] = v3[2];
            }
            ImGui.popID();
        }
        if (change) updateFunc.accept(firstChange);
    }

    private void vertexDataVec2(float[] data, int vertexCount, Consumer<Integer> updateFunc) {
        ImVec2 rcp = ImGui.getCursorPos();
        boolean change = false;
        int firstChange = 0;
        for (int i = 0; i < vertexCount; i++) {
            v2[0] = data[i * 2];
            v2[1] = data[i * 2 + 1];
            drawIndex(rcp, i);
            ImGui.pushID(i);
            ImGui.pushItemWidth(250);
            ImGui.setCursorPos(rcp.x + 50, rcp.y + 25 * i);
            boolean c = ImGui.dragFloat2("", v3, 0.01f);
            if (c) {
                if (!change) firstChange = i;
                change = true;
                data[i * 2] = v2[0];
                data[i * 2 + 1] = v2[1];
            }
            ImGui.popID();
        }
        if (change) updateFunc.accept(firstChange);
    }

    private void indexData(int[] indices, int faceCount, int drawFaces, Consumer<Integer> updateFunc) {
        ImVec2 rcp = ImGui.getCursorPos();
        boolean change = false;
        int firstChange = 0;
        for (int i = 0; i < faceCount; i++) {
            f3[0] = indices[i * 3];
            f3[1] = indices[i * 3 + 1];
            f3[2] = indices[i * 3 + 2];
            ImGui.pushStyleColor(ImGuiCol.Text, i >= drawFaces ? ImGui.getColorU32(0, 0, 0, 1) : ImGui.getColorU32(1, 1, 1, 1));
            drawIndex(rcp, i);
            ImGui.popStyleColor();
            ImGui.pushID(i);
            ImGui.pushItemWidth(250);
            ImGui.setCursorPos(rcp.x + 50, rcp.y + 25 * i);
            boolean c = ImGui.inputInt3("", f3);
            if (c) {
                if (!change) firstChange = i;
                change = true;
                indices[i * 3] = f3[0];
                indices[i * 3 + 1] = f3[1];
                indices[i * 3 + 2] = f3[2];
            }
            ImGui.popID();
        }
        if (change) updateFunc.accept(firstChange);
    }

    private void drawIndex(ImVec2 rcp, int i) {
        ImGui.setCursorPos(rcp.x + 15, rcp.y + 25 * i + 2);
        ImGui.text(Integer.toString(i));
        ImGui.sameLine();
    }
}
