package org.strobe.gfx.opengl.bindables.shader;

import java.util.HashMap;

public abstract class ShaderCompileConstantSet {

    private static EmptySet emptySet = null;

    public static ShaderCompileConstantSet getEmptySet(){
        if(emptySet == null)emptySet = new EmptySet();
        return emptySet;
    }

    private static final class EmptySet extends ShaderCompileConstantSet{
        private EmptySet(){
            super();
        }
    }

    private final HashMap<String, String> compileConstantMap = new HashMap<>();

    public ShaderCompileConstantSet(String...constantAllocations){
        for(String allocation : constantAllocations){
            String[] split = allocation.split("=");
            if(split.length!=2)throw new IllegalArgumentException("invalid shader compile constant layout");
            String constantName = split[0].trim();
            String constantValue = split[1];
            while(constantValue.contains(";")){
                constantValue = constantValue.substring(0, constantValue.lastIndexOf(";"));
            }
            constantValue = constantValue.trim();
            compileConstantMap.put(constantName, constantValue);
        }
    }

    public String getConstantValueString(String constantName){
        String value = compileConstantMap.get(constantName);
        if(value == null)throw new IllegalArgumentException("shader compile constant layout doesn't have constant " + constantName);
        return value;
    }

    public boolean isEmpty() {
        return compileConstantMap.isEmpty();
    }



}
