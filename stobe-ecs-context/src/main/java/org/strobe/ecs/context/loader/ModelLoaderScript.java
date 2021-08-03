package org.strobe.ecs.context.loader;

import org.joml.Vector3f;
import org.strobe.assimp.AssimpMesh;
import org.strobe.assimp.AssimpModelLoader;
import org.strobe.assimp.AssimpNode;
import org.strobe.ecs.ComponentMapper;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.materials.LambertianMaterial;
import org.strobe.ecs.context.renderer.materials.TestMaterial;
import org.strobe.ecs.context.renderer.mesh.Mesh;
import org.strobe.ecs.context.renderer.mesh.MeshRenderer;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.ecs.context.scripts.ScriptComponent;

public final class ModelLoaderScript extends ScriptComponent {

    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);

    private final String resource;

    public ModelLoaderScript(String resource) {
        this.resource = resource;
    }

    @Override
    protected void start() {
        AssimpNode rootNode = AssimpModelLoader.loadModel(resource);
        processNode(rootNode, getEntity());
    }

    private void processNode(AssimpNode node, Entity entity) {
        for (AssimpMesh assimpMesh : node.meshes()) {
            Entity meshChild = entity.createChild();
            //AssimpMesh -> Mesh (Component)
            int flag = 0;
            if (assimpMesh.hasPositions()) flag |= Mesh.ALLOCATE_POSITIONS;
            if (assimpMesh.hasTextureCoords()) flag |= Mesh.ALLOCATE_TEXTURE_COORDS;
            if (assimpMesh.hasNormals()) flag |= Mesh.ALLOCATE_NORMALS;
            if (assimpMesh.hasTangents()) flag |= Mesh.ALLOCATE_TANGENTS;
            if (assimpMesh.hasBitangents()) flag |= Mesh.ALLOCATE_TEXTURE_COORDS;
            Mesh mesh = new Mesh(assimpMesh.vertexCount(), assimpMesh.drawCount(), flag);
            if(assimpMesh.hasPositions())mesh.setPositions(0, assimpMesh.positions());
            if(assimpMesh.hasTextureCoords())mesh.setTextureCoords(0, assimpMesh.textureCoords());
            if(assimpMesh.hasNormals())mesh.setNormals(0, assimpMesh.normals());
            if(assimpMesh.hasTangents() && assimpMesh.hasBitangents())
                mesh.setTangents(0, assimpMesh.tangents(), assimpMesh.bitangents());
            mesh.setIndices(0, assimpMesh.indices());

            meshChild.addComponent(mesh);

            //AssimpMaterial -> Material (Component)
            meshChild.addComponent(new LambertianMaterial(new Vector3f(1,1,0)));

            meshChild.addComponent(new Transform());
            meshChild.addComponent(new FlagComponent());
            meshChild.addComponent(new MeshRenderer());
        }

        Transform transform = entity.get(TRANSFORM);
        if(transform == null){
            transform = new Transform(node.position(), node.scale(), node.orientation());
            entity.addComponent(transform);
        }else{
            //transform.setPosition(node.position());
            //transform.setScale(node.scale());
            //transform.setOrientation(node.orientation());
        }

        for(AssimpNode childNode : node.children()){
            Entity childEntity = entity.createChild();
            processNode(childNode, childEntity);
        }

    }

    @Override
    protected void update(float dt) {
    }
}
