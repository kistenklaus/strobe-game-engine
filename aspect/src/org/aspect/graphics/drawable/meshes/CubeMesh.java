package org.aspect.graphics.drawable.meshes;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Mesh;

public class CubeMesh extends Mesh {

    public CubeMesh(float xWidth, float yWidth, float zWidth) {
        super(Aspect.createVertexArray(true,
                new PrimitiveIbo.FillCube(),
                true,
                new PrimitiveVbo.CubePositions(xWidth, yWidth, zWidth),
                new PrimitiveVbo.CubeUVs(),
                new PrimitiveVbo.CubeNormals()),
                DrawMethod.ELEMENTS, DrawMode.TRIANGLES);
    }
}
