package org.aspect.graphics.drawable.meshes;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Mesh;

public class QuadMesh extends Mesh {
    public QuadMesh(float width, float height) {
        super(Aspect.createVertexArray(
                true, new PrimitiveIbo.FillQuad(),
                true,
                new PrimitiveVbo.QuadPositions(width, height), new PrimitiveVbo.QuadUVs()),
                DrawMethod.ELEMENTS,  DrawMode.TRIANGLES);
    }
}
