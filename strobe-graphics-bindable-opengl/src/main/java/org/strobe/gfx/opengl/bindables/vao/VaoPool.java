package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.BindablePool;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.UnconditionalUnbinding;

import java.util.ArrayList;
import java.util.Iterator;

public class VaoPool extends BindablePool<Vao> implements Debuggable {

    @Debug
    private final ArrayList<Vao> vaos = new ArrayList<>();

    public VaoPool() {
        //has to be unbinding!!!
        super(new UnconditionalUnbinding());
    }

    public void add(Vao vao){
        vaos.add(vao);
    }

    @Override
    public void dispose(Graphics gfx, Vao vao) {
        vaos.remove(vao);
        vao.dispose(gfx);
    }

    @Override
    public Iterator<Vao> iterator() {
        return vaos.iterator();
    }
}
