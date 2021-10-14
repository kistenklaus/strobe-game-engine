package org.strobe.gfx.opengl.bindables.util;

import org.joml.*;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

import java.lang.reflect.Type;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.GL_HALF_FLOAT;
import static org.lwjgl.opengl.GL41.GL_FIXED;

public final class TypeUtil {

    public static int getByteSizeOfGlTypeEnum(int glType) {
        //check if array of?
        return switch (glType) {
            case GL_BYTE -> 1;
            case GL_HALF_FLOAT, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED -> 2;
            case GL_FLOAT, GL_INT, GL_UNSIGNED_INT -> 4;
            case GL_DOUBLE -> 8;
            default -> throw new IllegalArgumentException();
        };
    }

    public static String getNameOfGlType(int glType){
        return switch(glType){
            case GL_BYTE -> "byte";
            case GL_HALF_FLOAT -> "half_float";
            case GL_SHORT -> "short";
            case GL_UNSIGNED_SHORT -> "ushort";
            case GL_FIXED -> "fixed";
            case GL_FLOAT -> "float";
            case GL_INT -> "int";
            case GL_UNSIGNED_INT -> "uint";
            case GL_DOUBLE -> "double";
            default -> throw new IllegalArgumentException();
        };
    }

    public static int getByteAlignmentOfGlslType(String glslType){
        return switch(glslType){
            case "int", "float" -> 4;
            case "vec2" -> 8;
            case "vec3", "mat4", "mat3", "vec4" -> 16;
            default -> throw new IllegalArgumentException(glslType + " is not a valid glsl type descriptor");
        };
    }

    public static int getByteSizeOfGlslType(String glslType){
        return switch(glslType){
            case "int","float"->4;
            case "vec2" -> 8;
            case "vec3" -> 12;
            case "vec4" -> 16;
            case "mat3" -> 48;
            case "mat4" -> 64;
            default -> throw new IllegalArgumentException("not a valid glsl type descriptor");
        };
    }

    public static Type glslTypeToJavaType(String glslType){
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

    public static Type glslTypeToJavaTypeArray(String glslType){
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

    public static int getByteSizeOfJavaType(Type type) {
        if(type.equals(Matrix4f.class))return 64;
        else if(type.equals(Vector4f.class))return 16;
        else if(type.equals(Vector3f.class))return 12;
        else if(type.equals(Integer.class)||type.equals(int.class))return 4;
        throw new IllegalArgumentException("getBytSizeJavaType doesn't support type : " + type);
    }
}
