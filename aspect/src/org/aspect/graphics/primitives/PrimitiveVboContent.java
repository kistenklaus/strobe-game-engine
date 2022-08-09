package org.aspect.graphics.primitives;

public abstract class PrimitiveVboContent {

    private final int size, stride, coverage, divisor;
    private final float[] content;

    protected PrimitiveVboContent(float[] content, int size, int stride, int coverage, int divisor) {
        this.content = content;
        this.size = size;
        this.stride = stride;
        this.coverage = coverage;
        this.divisor = divisor;
    }

    public float[] getVboContext() {
        return content;
    }

    public int getAttributeSize() {
        return size;
    }

    public int getAttributeStride() {
        return stride;
    }

    public int getAttributeCoverage() {
        return coverage;
    }

    public int getAttributeDivisor() {
        return divisor;
    }

    public static class QuadPositions extends PrimitiveVboContent {

        public QuadPositions(float width, float height) {
            super(new float[]{
                    -width / 2, height / 2, 0f,
                    width / 2, height / 2, 0f,
                    width / 2, -height / 2, 0f,
                    -width / 2, -height / 2, 0f
            }, 3, 3, 1, 0);
        }
    }

    public static class QuadUVs extends PrimitiveVboContent {
        private static float[] content = new float[]{
                0, 1,
                1, 1,
                1, 0,
                0, 0
        };

        public QuadUVs() {
            super(content, 2, 2, 1, 0);
        }
    }

    public static class CubePositions extends PrimitiveVboContent {

        public CubePositions(float xWidth, float yWidth, float zWidth) {
            super(new float[]{
                    //front
                    -xWidth / 2, -yWidth / 2, zWidth / 2,
                    -xWidth / 2, yWidth / 2, zWidth / 2,
                    xWidth / 2, yWidth / 2, zWidth / 2,
                    xWidth / 2, -yWidth / 2, zWidth / 2,
                    //right
                    -xWidth / 2, -yWidth / 2, -zWidth / 2,
                    -xWidth / 2, yWidth / 2, -zWidth / 2,
                    -xWidth / 2, yWidth / 2, zWidth / 2,
                    -xWidth / 2, -yWidth / 2, zWidth / 2,
                    //back
                    xWidth / 2, -yWidth / 2, -zWidth / 2,
                    xWidth / 2, yWidth / 2, -zWidth / 2,
                    -xWidth / 2, yWidth / 2, -zWidth / 2,
                    -xWidth / 2, -yWidth / 2, -zWidth / 2,
                    //left
                    xWidth / 2, -yWidth / 2, zWidth / 2,
                    xWidth / 2, yWidth / 2, zWidth / 2,
                    xWidth / 2, yWidth / 2, -zWidth / 2,
                    xWidth / 2, -yWidth / 2, -zWidth / 2,
                    //top
                    -xWidth / 2, yWidth / 2, zWidth / 2,
                    -xWidth / 2, yWidth / 2, -zWidth / 2,
                    xWidth / 2, yWidth / 2, -zWidth / 2,
                    xWidth / 2, yWidth / 2, zWidth / 2,
                    //bottom
                    xWidth / 2, -yWidth / 2, zWidth / 2,
                    xWidth / 2, -yWidth / 2, -zWidth / 2,
                    -xWidth / 2, -yWidth / 2, -zWidth / 2,
                    -xWidth / 2, -yWidth / 2, zWidth / 2,
            }, 3, 3, 1, 0);
        }
    }

    public static class CubeUVs extends PrimitiveVboContent {

        public static final float[] content = new float[]{
                //font face
                0, 0,
                0, 1,
                1, 1,
                1, 0,

                0, 0,
                0, 1,
                1, 1,
                1, 0,

                0, 0,
                0, 1,
                1, 1,
                1, 0,

                0, 0,
                0, 1,
                1, 1,
                1, 0,

                0, 0,
                0, 1,
                1, 1,
                1, 0,

                0, 0,
                0, 1,
                1, 1,
                1, 0,
        };

        public CubeUVs() {
            super(content, 2,2, 1, 0);
        }
    }

    public static class CubeNormals extends PrimitiveVboContent {

        public static final int size = 3;
        public static final int stride = 3;

        public static final float[] content = new float[]{
                0, 0, 1,
                0, 0, 1,
                0, 0, 1,
                0, 0, 1,

                -1, 0, 0,
                -1, 0, 0,
                -1, 0, 0,
                -1, 0, 0,

                0, 0, -1,
                0, 0, -1,
                0, 0, -1,
                0, 0, -1,

                1, 0, 0,
                1, 0, 0,
                1, 0, 0,
                1, 0, 0,

                0, 1, 0,
                0, 1, 0,
                0, 1, 0,
                0, 1, 0,

                0, -1, 0,
                0, -1, 0,
                0, -1, 0,
                0, -1, 0,
        };

        public CubeNormals() {
            super(content, 3, 3, 1, 0);
        }
    }
}