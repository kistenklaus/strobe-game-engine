package org.aspect.graphics.primitive;

public class Primitive {

    private final float[] content;

    protected Primitive(float[] content) {
        this.content = content;
    }

    public float[] getVboContext(){
        return content;
    }

    public static class QuadPositions extends Primitive{
        public static final int size = 3;
        public static final int stride = 3;
        public QuadPositions(float width, float height) {
            super(new float[]{
                    -width/2, height/2, 0f,
                    width/2, height/2, 0f,
                    width/2, -height/2, 0f,
                    -width/2, -height/2, 0f
            });
        }
    }

    public static class QuadUVs extends Primitive{
        public static final int size = 2;
        public static final int stride = 2;
        private static float[] content = new float[]{
                0, 1,
                1, 1,
                1, 0,
                0, 0
        };
        public QuadUVs() {
            super(content);
        }
    }
}
