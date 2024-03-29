package org.strobe.assimp;

public record AssimpMesh(String name, int vertexCount, float[] positions, float[] textureCoords,
                         float[] normals, float[] tangents, float[] bitangents,
                         int[] indices, int drawCount,
                         AssimpMaterial material) {

    public boolean hasPositions() {
        return positions != null;
    }

    public boolean hasTextureCoords() {
        return textureCoords != null;
    }

    public boolean hasNormals() {
        return normals != null;
    }

    public boolean hasTangents() {
        return tangents != null;
    }

    public boolean hasBitangents() {
        return tangents != null;
    }

}
