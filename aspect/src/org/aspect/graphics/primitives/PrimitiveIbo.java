package org.aspect.graphics.primitives;

public abstract class PrimitiveIbo {

    private final int[] content;

    protected PrimitiveIbo(int... content) {
        this.content = content;
    }

    public int[] getContent() {
        return content;
    }

    public static class FillQuad extends PrimitiveIbo{
        public FillQuad(){
            super(
              2,1,0,
              0,3,2
            );
        }
    }

    public static class FillCube extends PrimitiveIbo {
        public FillCube() {
            super(
                    2,1,0,
                    0,3,2,

                    6,5,4,
                    4,7,6,

                    10,9,8,
                    8,11,10,

                    14,13,12,
                    12,15,14,

                    18,17,16,
                    16,19,18,

                    22,21,20,
                    20,23,22
            );
        }
    }
}
