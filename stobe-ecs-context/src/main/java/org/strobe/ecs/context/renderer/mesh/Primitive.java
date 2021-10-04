package org.strobe.ecs.context.renderer.mesh;

public enum Primitive {
    XZ_PLANE(4, new float[]{
            -0.5f, 0f, 0.5f,
            -0.5f, 0f, -0.5f,
            0.5f, 0f, -0.5f,
            0.5f, 0f, 0.5f
    }, new float[]{
            0, 0,
            0, 1,
            1, 1,
            1, 0
    }, new float[]{
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0
    }, null, null, new int[]{
            2, 1, 0,
            0, 3, 2
    }),
    CUBE(4*6, new float[]{
            //front face
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            //back face
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            //right face
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            //left face
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            //top face
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,
            //bottom face
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,

    }, new float[]{
            //front face
            0,0,
            0,1,
            1,1,
            1,0,
            //back face
            1,0,
            1,0,
            1,1,
            0,0,
            //right face
            0,0,
            0,1,
            1,1,
            1,0,
            //left face
            1,0,
            1,1,
            0,1,
            1,1,
            //top face
            0,0,
            0,1,
            1,1,
            1,0,
            //bottom face
            0,1,
            0,0,
            1,0,
            1,1,
    }, new float[]{
            //front face
            0,0,1,
            0,0,1,
            0,0,1,
            0,0,1,
            //back face //fixme somehow 0,0,-1 doesn't work
            0,0,-1,
            0,0,-1,
            0,0,-1,
            0,0,-1,
            //right face
            1,0,0,
            1,0,0,
            1,0,0,
            1,0,0,
            //left face
            -1,0,0,
            -1,0,0,
            -1,0,0,
            -1,0,0,
            //top face
            0,1,0,
            0,1,0,
            0,1,0,
            0,1,0,
            //bottom face
            0,-1,0,
            0,-1,0,
            0,-1,0,
            0,-1,0,


    }, null,null, new int[]{
            2,1,0,
            0,3,2,

            4,5,6,
            6,7,4,

            10,9,8,
            8,11,10,

            12,13,14,
            14,15,12,

            16,17,18,
            18,19,16,

            22,21,20,
            20,23,22
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

    public int getVertexCount() {
        return vertexCount;
    }

    public int getDrawCount() {
        return indices.length;
    }
}
