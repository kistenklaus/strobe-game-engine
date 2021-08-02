package org.strobe.assimp;

import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.lwjgl.PointerBuffer;
import org.lwjgl.assimp.*;
import org.strobe.utils.ResourceLoader;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import static org.lwjgl.assimp.Assimp.*;
import static org.lwjgl.system.MemoryUtil.*;

public final class AssimpModelLoader {

    public static AssimpNode loadScene(String resource) {
        AIFileIO fileIo = AIFileIO.create()
                .OpenProc((pFileIO, fileName, openMode) -> {
                    ByteBuffer data;
                    String fileNameUtf8 = memUTF8(fileName);
                    try {
                        data = ResourceLoader.ioResourceToByteBuffer(fileNameUtf8, 8192);
                    } catch (IOException e) {
                        throw new RuntimeException("Could not open file: " + fileNameUtf8);
                    }
                    return AIFile.create()
                            .ReadProc((pFile, pBuffer, size, count) -> {
                                long max = Math.min(data.remaining(), size * count);
                                memCopy(memAddress(data) + data.position(), pBuffer, max);
                                return max;
                            })
                            .SeekProc((pFile, offset, origin) -> {
                                if (origin == Assimp.aiOrigin_CUR) {
                                    data.position(data.position() + (int) offset);
                                } else if (origin == Assimp.aiOrigin_SET) {
                                    data.position((int) offset);
                                } else if (origin == Assimp.aiOrigin_END) {
                                    data.position(data.limit() + (int) offset);
                                }
                                return 0;
                            })
                            .FileSizeProc(pFile -> data.limit())
                            .address();
                })
                .CloseProc((pFileIO, pFile) -> {
                    AIFile aiFile = AIFile.create(pFile);

                    aiFile.ReadProc().free();
                    aiFile.SeekProc().free();
                    aiFile.FileSizeProc().free();
                });
        AIScene scene = aiImportFileEx(resource,
                aiProcess_JoinIdenticalVertices | aiProcess_Triangulate, fileIo);
        fileIo.OpenProc().free();
        fileIo.CloseProc().free();
        if (scene == null) {
            throw new IllegalStateException(aiGetErrorString());
        }

        return processAIScene(scene);
    }

    private static AssimpMaterial[] processAISceneMaterials(AIScene scene) {
        AssimpMaterial[] materials = new AssimpMaterial[scene.mNumMaterials()];
        PointerBuffer aiMaterials = scene.mMaterials();
        for (int i = 0; i < scene.mNumMaterials(); i++) {
            AIMaterial aiMaterial = AIMaterial.create(aiMaterials.get(i));
            materials[i] = processAIMaterial(aiMaterial);
        }
        return materials;
    }

    private static AssimpMaterial processAIMaterial(AIMaterial aiMaterial) {
        String ambientTexturePath = null;
        String diffuseTexturePath = null;
        String specularTexturePath = null;
        String shininessTexturePath = null;
        String normalTexturePath = null;
        Vector3f ambientColor = null;
        Vector3f diffuseColor = null;
        Vector3f specularColor = null;
        Float shininessFactor = null;


        AIString string = AIString.calloc();

        if (aiGetMaterialTexture(aiMaterial, aiTextureType_AMBIENT, 0, string, (IntBuffer) null,
                null, null, null, null, null) == aiReturn_SUCCESS)
            ambientTexturePath = string.dataString();

        if (aiGetMaterialTexture(aiMaterial, aiTextureType_DIFFUSE, 0, string, (IntBuffer) null,
                null, null, null, null, null) == aiReturn_SUCCESS)
            diffuseTexturePath = string.dataString();

        if (aiGetMaterialTexture(aiMaterial, aiTextureType_SPECULAR, 0, string, (IntBuffer) null,
                null, null, null, null, null) == aiReturn_SUCCESS)
            specularTexturePath = string.dataString();

        if (aiGetMaterialTexture(aiMaterial, aiTextureType_SHININESS, 0, string, (IntBuffer) null,
                null, null, null, null, null) == aiReturn_SUCCESS)
            shininessTexturePath = string.dataString();

        if (aiGetMaterialTexture(aiMaterial, aiTextureType_HEIGHT, 0, string, (IntBuffer) null,
                null, null, null, null, null) == aiReturn_SUCCESS)
            normalTexturePath = string.dataString();

        AIColor4D aiColor = AIColor4D.calloc();
        if (aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_AMBIENT, aiTextureType_NONE, 0, aiColor) == aiReturn_SUCCESS) {
            ambientColor = new Vector3f(aiColor.r(), aiColor.g(), aiColor.b());
        }

        if (aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, aiTextureType_NONE, 0, aiColor) == aiReturn_SUCCESS) {
            diffuseColor = new Vector3f(aiColor.r(), aiColor.g(), aiColor.b());
        }

        if (aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_SPECULAR, aiTextureType_NONE, 0, aiColor) == aiReturn_SUCCESS) {
            specularColor = new Vector3f(aiColor.r(), aiColor.g(), aiColor.b());
        }

        return new AssimpMaterial(ambientTexturePath, diffuseTexturePath, specularTexturePath,
                shininessTexturePath, normalTexturePath, ambientColor, diffuseColor, specularColor, shininessFactor);
    }

    private static AssimpMesh[] processAISceneMeshes(AIScene scene) {
        return processAISceneMeshes(scene, processAISceneMaterials(scene));
    }

    private static AssimpMesh[] processAISceneMeshes(AIScene scene, AssimpMaterial[] materials) {
        AssimpMesh[] meshes = new AssimpMesh[scene.mNumMeshes()];
        PointerBuffer aiMeshes = scene.mMeshes();
        for (int i = 0; i < scene.mNumMeshes(); i++) {
            AIMesh aiMesh = AIMesh.create(aiMeshes.get(i));
            meshes[i] = processAIMesh(aiMesh, materials);
        }
        return meshes;
    }

    private static AssimpMesh processAIMesh(AIMesh aiMesh, AssimpMaterial[] materials) {
        int vertexCount = aiMesh.mNumVertices();

        AIVector3D.Buffer aiPositions = aiMesh.mVertices();
        AIVector3D.Buffer aiTextureCoords = aiMesh.mTextureCoords(0);
        AIVector3D.Buffer aiNormals = aiMesh.mNormals();
        AIVector3D.Buffer aiTangents = aiMesh.mTangents();
        AIVector3D.Buffer aiBitangents = aiMesh.mBitangents();

        boolean hasPositions = aiPositions != null;
        boolean hasTextureCoords = aiTextureCoords != null;
        boolean hasNormals = aiNormals != null;
        boolean hasTangents = aiTangents != null;
        boolean hasBitangents = aiBitangents != null;

        float[] positions = hasPositions ? new float[vertexCount * 3] : null;
        float[] textureCoords = hasTextureCoords ? new float[vertexCount*2]:null;
        float[] normals = hasNormals ? new float[vertexCount*3]:null;
        float[] tangents = hasTangents ? new float[vertexCount*3]:null;
        float[] bitangents = hasBitangents ? new float[vertexCount*3]:null;

        if(hasPositions){
            int i=0;
            while(aiPositions.hasRemaining()){
                AIVector3D vec3 = aiPositions.get();
                positions[i++] = vec3.x();
                positions[i++] = vec3.y();
                positions[i++] = vec3.z();
            }
        }

        if(hasTextureCoords){
            int i=0;
            while(aiTextureCoords.hasRemaining()){
                AIVector3D vec3 = aiTextureCoords.get();
                textureCoords[i++] = vec3.x();
                textureCoords[i++] = vec3.y();
            }
        }

        if(hasNormals){
            int i=0;
            while(aiNormals.hasRemaining()){
                AIVector3D vec3 = aiNormals.get();
                normals[i++] = vec3.x();
                normals[i++] = vec3.y();
                normals[i++] = vec3.z();
            }
        }

        if(hasTangents){
            int i=0;
            while(aiTangents.hasRemaining()){
                AIVector3D vec3 = aiTangents.get();
                tangents[i++] = vec3.x();
                tangents[i++] = vec3.y();
                tangents[i++] = vec3.z();
            }
        }

        if(hasBitangents){
            int i=0;
            while(aiBitangents.hasRemaining()){
                AIVector3D vec3 = aiBitangents.get();
                bitangents[i++] = vec3.x();
                bitangents[i++] = vec3.y();
                bitangents[i++] = vec3.z();
            }
        }

        AssimpMaterial material = materials[aiMesh.mMaterialIndex()];
        String name = aiMesh.mName().dataString();

        return new AssimpMesh(name, positions, textureCoords, normals, tangents, bitangents, material);
    }

    private static AssimpNode processAINode(AINode aiNode, AssimpMesh[] meshes) {

        AIMatrix4x4 aiTransform = aiNode.mTransformation();
        Matrix4f transform = new Matrix4f(
                aiTransform.a1(), aiTransform.a2(), aiTransform.a3(), aiTransform.a4(),
                aiTransform.b1(), aiTransform.b2(), aiTransform.b3(), aiTransform.b4(),
                aiTransform.c1(), aiTransform.c2(), aiTransform.c3(), aiTransform.c4(),
                aiTransform.d1(), aiTransform.d2(), aiTransform.d3(), aiTransform.d4());
        Vector3f position = new Vector3f();
        transform.getTranslation(position);
        Vector3f scale = new Vector3f();
        transform.getScale(scale);
        Quaternionf orientation = new Quaternionf();
        transform.getNormalizedRotation(orientation);

        AssimpMesh[] nodeMeshes = new AssimpMesh[aiNode.mNumMeshes()];
        IntBuffer meshIndices = aiNode.mMeshes();
        if(meshIndices != null){
            int i=0;
            while(meshIndices.hasRemaining())nodeMeshes[i++] = meshes[meshIndices.get()];
        }

        AssimpNode[] children = new AssimpNode[aiNode.mNumChildren()];
        PointerBuffer aiChildren = aiNode.mChildren();
        if(aiChildren == null){
            int i=0;
            while(aiChildren.hasRemaining())
                children[i++] = processAINode(AINode.create(aiChildren.get()), meshes);
        }

        String name = aiNode.mName().dataString();
        return new AssimpNode(name, position, scale, orientation, nodeMeshes, children);
    }

    private static AssimpNode processAIScene(AIScene scene){
        AssimpMesh[] meshes = processAISceneMeshes(scene);
        return processAINode(scene.mRootNode(), meshes);
    }
}
