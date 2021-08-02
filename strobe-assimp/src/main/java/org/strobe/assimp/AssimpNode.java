package org.strobe.assimp;

import org.joml.Quaternionf;
import org.joml.Vector3f;

public record AssimpNode(String name, Vector3f position, Vector3f scale, Quaternionf orientation,
                         AssimpMesh[] meshes, AssimpNode[] children) {
}
