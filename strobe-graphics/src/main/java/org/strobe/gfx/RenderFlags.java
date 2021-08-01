package org.strobe.gfx;

public final class RenderFlags {

    public static final int BLEND = 0b1;
    public static final int DEFERRED = BLEND>>1;

    public static boolean containsFlag(int flag, int toCheck){
        return (flag & toCheck) != 0;
    }

}
