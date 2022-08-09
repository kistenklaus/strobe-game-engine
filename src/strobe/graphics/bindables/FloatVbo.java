package strobe.graphics.bindables;

import aspect.Aspect;
import aspect.AspectBindable;
import strobe.StrobeException;

import java.util.Arrays;

import static org.lwjgl.opengl.GL15.*;

//TODO generic VBO
public class FloatVbo extends AspectBindable {

    private int ID;

    private final int allocated_size;
    private final float[] content;
    private final int usage;
    private boolean updated = false;
    private int scheduled_buffer_offset = 0;
    private boolean bound = false;

    public FloatVbo(float[] content,  int usage){
        this(content, content.length, usage);
    }
    public FloatVbo(int allocated_size, int usage){
        this(new float[allocated_size], allocated_size, usage);
    }
    public FloatVbo(float[] content, int allocated_size, int usage){
        this.content = content;
        this.allocated_size = allocated_size;
        this.usage = usage;
        if(this.allocated_size == 0) throw new StrobeException("VBO size can't be 0");
        if(usage!=GL_STATIC_DRAW&&usage!=GL_DYNAMIC_DRAW) throw new StrobeException("VBO usage is illegal");
    }

    @Override
    public final void create() {
        if(!Thread.currentThread().getName().equals(Aspect.getThreadName())) throw new StrobeException("Illegal Thread");
        ID = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, allocated_size * Float.BYTES, usage);
        glBufferSubData(GL_ARRAY_BUFFER, 0, content);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        updated = true;
    }

    public final synchronized void buffer(int offset, float[] content){
        for(int i=offset;i<Math.min(content.length, this.allocated_size);i++)this.content[i] = content[i];
        updated = true;
        this.scheduled_buffer_offset = Math.min(this.scheduled_buffer_offset, offset);
        if(Thread.currentThread().getName() == Aspect.getThreadName())dobuffer();
    }

    public final synchronized void buffer(float[] content){
        buffer(0, content);
    }

    @Override
    public final synchronized void bind() {
        dobuffer();
        bound = true;
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }

    /**
     * buffers the scheduled changes
     * @implNote unbinds all buffers bound to the array buffer
     */
    public final void dobuffer(){
        if(updated && !bound){
            glBindBuffer(GL_ARRAY_BUFFER, ID);
            glBufferSubData(GL_ARRAY_BUFFER, this.scheduled_buffer_offset * Float.BYTES, content);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    @Override
    public final synchronized void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        bound = false;
    }

    @Override
    public final void dispose() {
        glDeleteBuffers(ID);
    }

    public final int getID(){
        return ID;
    }

    public final int getSize(){
        return allocated_size;
    }
}
