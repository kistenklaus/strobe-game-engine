package org.strobe.ecs.context.renderer.mesh;

import org.strobe.ecs.Component;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.vao.Ibo;
import org.strobe.gfx.opengl.bindables.vao.Vao;
import org.strobe.gfx.renderables.opengl.IndexedVao;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.function.BiConsumer;

import static org.lwjgl.opengl.GL11.GL_TRIANGLES;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_INT;

public class Mesh implements Component {

    public static final int ALLOCATE_POSITIONS = 0x1;
    public static final int ALLOCATE_TEXTURE_COORDS = 0x2;
    public static final int ALLOCATE_NORMALS = 0x4;
    public static final int ALLOCATE_TANGENTS = 0x8;
    public static final int DYNAMIC_POSITIONS = ALLOCATE_POSITIONS | 0x10;
    public static final int DYNAMIC_TEXTURE_COORDS = ALLOCATE_TEXTURE_COORDS | 0x20;
    public static final int DYNAMIC_NORMALS = ALLOCATE_NORMALS | 0x40;
    public static final int DYNAMIC_TANGENTS = ALLOCATE_TANGENTS | 0x80;
    public static final int DYNAMIC_INDICES = 0x100;

    private final float[] positions;
    private final float[] textureCoords;
    private final float[] normals;
    private final float[] tangents;
    private final float[] bitangents;

    private final int[] indices;

    private final int vertexCount;
    private final int drawCount;

    protected final LinkedList<BiConsumer<Graphics, EntityRenderer>> meshOps = new LinkedList<>();

    private final String[] layout;

    private final boolean dynamicIndices;
    private final boolean dynamicPositions;
    private final boolean dynamicUVs;
    private final boolean dynamicNormals;
    private final boolean dynamicTangents;

    private Vao vao = null;
    private Ibo ibo = null;

    public Mesh(int vertexCount, int indicesCount, int drawCount, int flag) {
        this.vertexCount = vertexCount;
        this.drawCount = drawCount;

        boolean hasPositions = (flag & ALLOCATE_POSITIONS) == ALLOCATE_POSITIONS;
        boolean hasUVs = (flag & ALLOCATE_TEXTURE_COORDS) == ALLOCATE_TEXTURE_COORDS;
        boolean hasNormals = (flag & ALLOCATE_NORMALS) == ALLOCATE_NORMALS;
        boolean hasTangents = (flag & ALLOCATE_TANGENTS) == ALLOCATE_TANGENTS;
        dynamicIndices = (flag & DYNAMIC_INDICES) == DYNAMIC_INDICES;
        dynamicPositions = (flag & DYNAMIC_POSITIONS) == DYNAMIC_POSITIONS;
        dynamicUVs = (flag & DYNAMIC_TEXTURE_COORDS) == DYNAMIC_TEXTURE_COORDS;
        dynamicNormals = (flag & DYNAMIC_NORMALS) == DYNAMIC_NORMALS;
        dynamicTangents = (flag & DYNAMIC_TANGENTS) == DYNAMIC_TANGENTS;
        StringBuilder layoutStr = new StringBuilder();
        boolean first = true;
        if (hasPositions) {
            layoutStr.append("layout(location=0,usage=").append(dynamicPositions ? "dynamic" : "static")
                    .append(") in vec3");
            positions = new float[vertexCount * 3];
            first = false;
        } else positions = null;
        if (hasUVs) {
            if (!first) layoutStr.append("\n");
            layoutStr.append("layout(location=1,usage=").append(dynamicUVs ? "dynamic" : "static")
                    .append(") in vec2");
            textureCoords = new float[vertexCount * 2];
        } else textureCoords = null;
        if (hasNormals) {
            if (!first) layoutStr.append("\n");
            layoutStr.append("layout(location=2,usage=").append(dynamicNormals ? "dynamic" : "static")
                    .append(") in vec3");
            normals = new float[vertexCount * 3];
        } else normals = null;
        if (hasTangents) {
            if (!first) layoutStr.append("\n");
            String usage = dynamicTangents ? "dynamic" : "static";
            layoutStr.append("layout(location=3,usage=").append(usage).append(") in vec3").append("\n")
                    .append("layout(location=4,usage=").append(usage).append(") in vec3");
            tangents = new float[vertexCount * 3];
            bitangents = new float[vertexCount * 3];
        } else {
            tangents = null;
            bitangents = null;
        }
        layout = layoutStr.toString().split("\n");

        indices = new int[indicesCount];
    }

    public Mesh(int vertexCount, int drawCount, int flags) {
        this(vertexCount, drawCount, drawCount, flags);
    }

    public Mesh(int vertexCount, int drawCount) {
        this(vertexCount, drawCount, DYNAMIC_POSITIONS | DYNAMIC_NORMALS);
    }

    public Mesh(Primitive primitive, int flags){
        this(primitive.getVertexCount(), primitive.getDrawCount(), flags);
        System.out.println(Arrays.toString(primitive.getPositions()));
        if(positions != null && primitive.getPositions()!=null)
            setPositions(0, primitive.getPositions());
        if(textureCoords != null && primitive.getTextureCoords()!=null)
            setTextureCoords(0, primitive.getTextureCoords());
        if(normals != null && primitive.getNormals()!=null)
            setNormals(0, primitive.getNormals());
        if(tangents != null && primitive.getTangents() != null
            &&bitangents!=null && primitive.getBitangents() != null)
            setTangents(0, primitive.getTangents(), primitive.getBitangents());
        setIndices(0, primitive.getIndices());
    }

    void enqueueScheduledMeshOps(EntityRenderer renderer) {
        if (meshOps.isEmpty()) return;
        renderer.enqueueRenderOps(meshOps);
        meshOps.clear();
    }

    Renderable createRenderable(Graphics gfx) {
        if (vao != null) return new IndexedVao(vao, getDrawCount());
        meshOps.clear();
        vao = new Vao(gfx, vertexCount, ibo = new Ibo(gfx, getIndices(), dynamicIndices), layout);
        System.out.println(Arrays.toString(positions));
        if (positions != null) vao.bufferLocation(gfx, 0, positions);
        if (textureCoords != null) vao.bufferLocation(gfx, 1, textureCoords);
        if (normals != null) vao.bufferLocation(gfx, 2, normals);
        if (tangents != null) vao.bufferLocation(gfx, 3, tangents);
        if (bitangents != null) vao.bufferLocation(gfx, 4, bitangents);
        return new IndexedVao(vao, getDrawCount());
    }

    public int getVertexCount() {
        return vertexCount;
    }

    public int getDrawCount() {
        return drawCount;
    }

    public int getIndicesCount() {
        return indices.length;
    }

    public void setPositions(int start, float[] positions) {
        if (this.positions == null) throw new IllegalStateException();
        start *= 3;
        if (positions.length % 3 != 0) throw new IllegalArgumentException();
        if (positions.length + start > this.positions.length) throw new IllegalArgumentException();
        for (int i = 0; i < positions.length; i++) this.positions[i + start] = positions[i];
        meshOps.add((gfx,renderer) -> {
            if (vao != null) vao.bufferLocation(gfx, 0, this.positions);
        });
    }

    public void setTextureCoords(int start, float[] textureCoords) {
        if (this.textureCoords == null) throw new IllegalStateException();
        start *= 2;
        if (textureCoords.length % 2 != 0) throw new IllegalArgumentException();
        if (textureCoords.length + start > this.textureCoords.length) throw new IllegalArgumentException();
        for (int i = 0; i < textureCoords.length; i++) this.textureCoords[i + start] = textureCoords[i];
        meshOps.add((gfx,renderer) -> {
            if (vao != null) vao.bufferLocation(gfx, 1, this.textureCoords);
        });
    }

    public void setNormals(int start, float[] normals) {
        if (this.normals == null) throw new IllegalStateException();
        start *= 3;
        if (normals.length % 3 != 0) throw new IllegalArgumentException();
        if (normals.length + start > this.normals.length) throw new IllegalArgumentException();
        for (int i = 0; i < normals.length; i++) this.normals[i + start] = normals[i];
        meshOps.add((gfx,renderer) -> {
            if (vao != null) vao.bufferLocation(gfx, 2, this.normals);
        });
    }

    public void setTangents(int start, float[] tangents, float[] bitangents) {
        if (this.tangents == null || this.bitangents == null) throw new IllegalStateException();
        start *= 3;
        if (tangents.length % 3 != 0 || bitangents.length % 3 != 0) throw new IllegalArgumentException();
        if (tangents.length != bitangents.length) throw new IllegalArgumentException();
        if (tangents.length + start > this.tangents.length) throw new IllegalArgumentException();
        for (int i = 0; i < tangents.length; i++) {
            this.tangents[i + start] = tangents[i];
            this.bitangents[i + start] = bitangents[i];
        }
        meshOps.add((gfx,renderer) -> {
            if (vao != null) {
                vao.bufferLocation(gfx, 3, this.tangents);
                vao.bufferLocation(gfx, 4, this.bitangents);
            }
        });
    }


    public void setIndices(int start, int[] indices) {
        if (start % 3 != 0) throw new IllegalArgumentException();
        if (indices.length % 3 != 0) throw new IllegalArgumentException();
        if (indices.length + start > this.indices.length) throw new IllegalArgumentException();
        for (int i = 0; i < indices.length; i++) this.indices[i + start] = indices[i];
        meshOps.add((gfx,renderer) -> {
            if (vao != null) ibo.bufferSubData(gfx, start, indices);
        });
    }

    public float[] getPositions() {
        return positions;
    }

    public float[] getTextureCoords() {
        return textureCoords;
    }

    public float[] getNormals() {
        return normals;
    }

    public float[] getTangents() {
        return tangents;
    }

    public float[] getBitangents() {
        return bitangents;
    }

    public int[] getIndices() {
        return indices;
    }

}
