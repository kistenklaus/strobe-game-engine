package org.aspect.graphics.bindable;

import java.util.Arrays;

import static org.lwjgl.opengl.GL15.*;

public final class VertexBuffer extends Bindable {

    private int ID;
    private int allocated_floats;
    //should not be field
    private float[] content;
    private boolean readonly;

    private static int hashAcc = 0;
    private final String hash;

    public VertexBuffer(float[] content, int allocated_floats, boolean readonly){
        if(readonly){
            StringBuilder hashStr = new StringBuilder();
            hashStr.append(getClass()).append(":")
                    .append(Arrays.toString(content)).append(":")
                    .append(allocated_floats).append(":")
                    .append("readonly");
            hash = hashStr.toString();
        }else{
            hash = getClass()+":dynamicDraw:"+hashAcc++;
        }


        this.content = content;
        this.allocated_floats = allocated_floats;
        this.readonly = readonly;
    }

    public void create() {

        ID = glGenBuffers();

        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, allocated_floats * Float.BYTES,
                readonly ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, content);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    @Override
    public void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }

    @Override
    public void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    @Override
    public void dispose() {
        unbind();
        glDeleteBuffers(ID);
    }

    public int getSize(){
        return allocated_floats;
    }

    public boolean isReadonly(){
        return readonly;
    }

    public String hash(){
        return hash;
    }
}
