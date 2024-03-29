package org.strobe.ecs.context.renderer.mesh;

import org.strobe.ecs.Component;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.ecs.context.renderer.materials.Material;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.gfx.rendergraph.core.GraphDrawable;
import org.strobe.gfx.rendergraph.core.Technique;

public final class MeshRenderer implements Component {


    protected final GraphDrawable graphDrawable = new GraphDrawable(null, null);
    private Mesh currentMesh = null;

    private Technique materialTechnique = null;

    public MeshRenderer() {

    }


    void update(EntityRenderer renderer, Transform transform, Mesh mesh, Material material) {
        graphDrawable.setTransform(transform);

        //create meshes
        if (mesh != currentMesh) {
            currentMesh = mesh;
            if (currentMesh != null) {
                //create AbstractMesh
                renderer.enqueueRenderOp((gfx,r) -> {
                    if (currentMesh == null) {
                        graphDrawable.setRenderable(null);
                    } else {
                        graphDrawable.setRenderable(currentMesh.createRenderable(gfx));
                    }
                });
            } else {
                renderer.enqueueRenderOp((gfx,r) -> graphDrawable.setRenderable(null));
            }
        }
        currentMesh.enqueueScheduledMeshOps(renderer);

        Technique matTech = material != null ? material.getTechnique() : null;
        if (materialTechnique == null && matTech != null) {
            graphDrawable.addTechnique(matTech);
            materialTechnique = matTech;
        } else if (materialTechnique != null && matTech == null) {
            graphDrawable.removeTechnique(materialTechnique);
            materialTechnique = null;
        } else if (materialTechnique != null && matTech != null) {
            if (materialTechnique != matTech){
                graphDrawable.removeTechnique(materialTechnique);
                graphDrawable.addTechnique(matTech);
                materialTechnique = matTech;
            }
        }

    }

}
