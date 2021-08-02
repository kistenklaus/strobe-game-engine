package org.strobe.gfx.renderables.opengl;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.vao.Vao;

import static org.lwjgl.opengl.GL11.*;

public class IndexedVao implements Renderable {

    private final Vao vao;
    private int drawCount;
    private final int mode;

    public IndexedVao(Vao vao, int drawCount, int mode){
        this.vao = vao;
        this.drawCount = drawCount;
        this.mode = mode;
    }
    public IndexedVao(Vao vao, int drawCount){
        this(vao, drawCount, GL_TRIANGLES);
    }

    @Override
    public void render(Graphics gfx) {
        gfx.bind(vao);
        glDrawElements(mode, drawCount, GL_UNSIGNED_INT, 0);
        gfx.unbind(vao);
    }
}
