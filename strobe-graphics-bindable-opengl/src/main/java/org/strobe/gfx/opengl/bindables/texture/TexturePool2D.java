package org.strobe.gfx.opengl.bindables.texture;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.BindablePool;
import org.strobe.gfx.UnconditionalUnbinding;

import java.util.ArrayList;
import java.util.Iterator;

public class TexturePool2D extends BindablePool<Texture2D> implements Debuggable {

    @Debug
    private final ArrayList<Texture2D> textures = new ArrayList<>();

    public TexturePool2D() {
        super(new UnconditionalUnbinding());
    }

    public void add(Texture2D tex){
        this.textures.add(tex);
    }

    @Override
    public Iterator<Texture2D> iterator() {
        return textures.iterator();
    }
}
