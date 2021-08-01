package org.strobe.debug;


import org.joml.Options;
import org.joml.Runtime;

public final class NumberFormatUtil {

    public static String formatDouble(double value){
        String normal = Double.toString(value);
        if(normal.length() < 8)return normal;
        return Runtime.format(value, Options.NUMBER_FORMAT);
    }

    public static String formatLong(long value){
        String normal = Long.toString(value);
        //scientific notation
        return normal;
    }
}
