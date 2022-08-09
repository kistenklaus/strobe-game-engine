package org.aspect.graphics.drawable.meshes;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Mesh;

public class CubeMesh extends Mesh {

    public CubeMesh(float xWidth, float yWidth, float zWidth) {
        super(Aspect.createVertexArray(true,
                new PrimitiveIboContent.FillCube(),
                true,
                new PrimitiveVboContent.CubePositions(xWidth, yWidth, zWidth),
                new PrimitiveVboContent.CubeUVs(),
                new PrimitiveVboContent.CubeNormals()),
                DrawMethod.ELEMENTS, DrawMode.TRIANGLES);
    }
}
