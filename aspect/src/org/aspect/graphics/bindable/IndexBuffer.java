package org.aspect.graphics.bindable;

import org.aspect.graphics.drawable.attachments.IndexBufferAttachment;

import java.util.Arrays;

import static org.lwjgl.opengl.GL15.*;

public class IndexBuffer extends Bindable {

    private int ID;
    private int[] content;
    private int allocated_ints;
    private boolean readonly;

    private static int hashAcc = 0;
    private final String hash;

    public IndexBuffer(int[] content, int allocated_ints, boolean readonly){
        if(readonly){
            StringBuilder hashStr = new StringBuilder();
            hashStr.append(this.getClass())
                    .append(Arrays.toString(content))
                    .append(",size:").append(allocated_ints)
                    .append(",readonly");
            hash = hashStr.toString();
        }else{
            hash = this.getClass()+":dynamicDraw:"+hashAcc++;
        }
        this.content = content;
        this.allocated_ints = allocated_ints;
        this.readonly = readonly;
    }

    public void create() {
        ID = glGenBuffers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, allocated_ints * Integer.BYTES,
                readonly?GL_STATIC_DRAW:GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, content);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    @Override
    public void bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    }

    @Override
    public void unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    @Override
    public void dispose() {
        glDeleteBuffers(ID);
    }

    public int getSize(){
        return allocated_ints;
    }

    public boolean isReadonly(){
        return readonly;
    }

    public String hash(){
        return hash;
    }

    public IndexBufferAttachment makeAttachment(){
        return new IndexBufferAttachment(this);
    }
}
