package org.strobe.gfx.opengl.bindables.util;

import org.joml.*;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

import java.lang.reflect.Type;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.GL_HALF_FLOAT;
import static org.lwjgl.opengl.GL41.GL_FIXED;

public final class TypeUtil {

    public static int getSizeOfGlTypeEnum(int glType) {
        //check if array of?
        return switch (glType) {
            case GL_BYTE -> 1;
            case GL_HALF_FLOAT, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED -> 2;
            case GL_FLOAT, GL_INT, GL_UNSIGNED_INT -> 4;
            case GL_DOUBLE -> 8;
            default -> throw new IllegalArgumentException();
        };
    }

    public static int getAlignmentOfGlslType(String glslType){
        return switch(glslType){
            case "int", "float" -> 4;
            case "vec2" -> 8;
            case "vec3", "mat4", "mat3", "vec4" -> 16;
            default -> throw new IllegalArgumentException(glslType + " is not a valid glsl type descriptor");
        };
    }

    public static int getSizeOfGlslType(String glslType){
        return switch(glslType){
            case "int","float"->4;
            case "vec2" -> 8;
            case "vec3" -> 12;
            case "vec4" -> 16;
            case "mat4" -> 64;
            case "mat3" -> 48;
            default -> throw new IllegalArgumentException("not a valid glsl type descriptor");
        };
    }

    public static Type glslTypeToClass(String glslType){
        return switch(glslType){
            case "int" -> int.class;
            case "bool" -> boolean.class;
            case "float" -> float.class;
            case "vec2" -> Vector2f.class;
            case "vec3" -> Vector3f.class;
            case "vec4" -> Vector4f.class;
            case "mat4" -> Matrix4f.class;
            case "mat3" -> Matrix3f.class;
            case "sampler2D" -> Texture2D.class;
            default -> throw new IllegalArgumentException(glslType + " is not a valid glsl type descriptor");
        };
    }

    public static Type glslTypeToClassArray(String glslType){
        return switch(glslType){
            case "int" -> int[].class;
            case "float" -> float[].class;
            case "vec2" -> Vector2f[].class;
            case "vec3" -> Vector3f[].class;
            case "vec4" -> Vector4f[].class;
            case "mat4" -> Matrix4f[].class;
            case "mat3" -> Matrix3f[].class;
            case "sampler2D" -> Texture2D[].class;
            default -> throw new IllegalArgumentException("not a valid glsl type descriptor");
        };
    }

}
