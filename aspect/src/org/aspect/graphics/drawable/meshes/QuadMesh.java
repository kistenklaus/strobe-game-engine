package org.aspect.graphics.drawable.meshes;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Mesh;

public class QuadMesh extends Mesh {
    public QuadMesh(float width, float height) {
        super(Aspect.createVertexArray(
                true, new PrimitiveIboContent.FillQuad(),
                true,
                new PrimitiveVboContent.QuadPositions(width, height), new PrimitiveVboContent.QuadUVs()),
                DrawMethod.ELEMENTS,  DrawMode.TRIANGLES);
    }
}
