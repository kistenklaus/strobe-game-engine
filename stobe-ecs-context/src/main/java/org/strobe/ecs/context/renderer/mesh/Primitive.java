package org.strobe.ecs.context.renderer.mesh;

public enum Primitive {
    XZ_QUAD(4, new float[]{
            -0.5f, 0,0.5f,
            -0.5f, 0f,-0.5f,
            0.5f, 0f -0.5f,
            0.5f, 0f,0.5f
    }, new float[]{
            0,0,
            0,1,
            1,1,
            1,0
    }, new float[]{
            0,1,0,
            0,1,0,
            0,1,0,
            0,1,0
    },null,null,  new int[]{
            2,1,0,
            0,3,2
    });

    private final int vertexCount;
    private final float[] positions;
    private final float[] textureCoords;
    private final float[] normals;
    private final float[] tangents;
    private final float[] bitangents;
    private final int[] indices;

    Primitive(int vertexCount, float[] positions,
              float[] textureCoords, float[] normals,
              float[] tangents, float[] bitangents,
              int[] indices) {
        this.vertexCount = vertexCount;
        this.positions = positions;
        this.textureCoords = textureCoords;
        this.normals = normals;
        this.tangents = tangents;
        this.bitangents = bitangents;
        this.indices = indices;
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

    public int getVertexCount(){
        return vertexCount;
    }

    public int getDrawCount(){
        return indices.length;
    }
}
