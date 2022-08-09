package org.aspect.graphics.drawable;

import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;

public class Mesh {

    private VertexArray vao;

    private DrawMethod method;
    private DrawMode mode;

    private int drawCount;
    private int instanceCount;


    public Mesh(VertexArray vao, DrawMethod method, DrawMode mode){
        this.vao = vao;
        this.drawCount = this.vao.getIbo().getSize();
        this.instanceCount = 1;
        this.method = method;
        this.mode = mode;
    }

    public VertexArray getVao() {
        return vao;
    }

    public DrawMethod getMethod() {
        return method;
    }

    public DrawMode getMode() {
        return mode;
    }

    public int getDrawCount(){
        return this.drawCount;
    }

    public int getInstanceCount(){
        return this.instanceCount;
    }
}
