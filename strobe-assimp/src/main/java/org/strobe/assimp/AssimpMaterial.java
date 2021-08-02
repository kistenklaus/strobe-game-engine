package org.strobe.assimp;

import org.joml.Vector3f;

public record AssimpMaterial(String ambientTexturePath, String diffuseTexturePath,
                             String specularTexturePath, String shininessTexturePath,
                             String normalTexturePath, Vector3f ambientColor,
                             Vector3f diffuseColor, Vector3f specularColor,
                             Float shininessFactor) {

    public boolean hasAmbientTexture() {
        return ambientTexturePath != null;
    }

    public boolean hasDiffuseTexture() {
        return diffuseTexturePath != null;
    }

    public boolean hasSpecularTexture() {
        return specularTexturePath != null;
    }

    public boolean hasShininessTexture() {
        return shininessTexturePath != null;
    }

    public boolean haNormalTexture() {
        return normalTexturePath != null;
    }

    public boolean hasAmbientColor() {
        return ambientColor != null;
    }

    public boolean hasDiffuseColor() {
        return diffuseColor != null;
    }

    public boolean hasSpecularColor() {
        return specularColor != null;
    }

    public boolean hasShininessFactor() {
        return shininessFactor != null;
    }
}
