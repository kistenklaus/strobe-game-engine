package org.strobe.game;

import org.joml.Vector3f;
import org.strobe.ecs.context.renderer.materials.LambertianMaterial;
import org.strobe.ecs.context.renderer.mesh.Mesh;
import org.strobe.ecs.context.renderer.mesh.MeshRenderer;
import org.strobe.ecs.context.renderer.mesh.Primitive;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.ecs.context.scripts.ScriptComponent;

public class GroundGenScript extends ScriptComponent {


    @Override
    protected void start() {
        Mesh mesh = new Mesh(Primitive.XZ_PLANE,
                Mesh.ALLOCATE_POSITIONS | Mesh.ALLOCATE_NORMALS);
        getEntity().addComponent(mesh);
        getEntity().addComponent(new MeshRenderer());
        getEntity().addComponent(new LambertianMaterial(new Vector3f(0.25f)));
        getEntity().addComponent(new Transform(null,
                new Vector3f(100), null));
    }
}
