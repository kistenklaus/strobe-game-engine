package org.aspect.graphics.bindable;

import org.joml.Matrix4f;
import org.joml.Vector4f;

public abstract class Shader extends Bindable {
    public abstract void create(String vertexSrc, String fragmentSrc);

    public abstract void uniformMat4(String uniformName, Matrix4f mat4);
    public abstract void uniformVec4(String uniformName, Vector4f vec4);
    public abstract void uniform1i(String uniformName, int i);
}
