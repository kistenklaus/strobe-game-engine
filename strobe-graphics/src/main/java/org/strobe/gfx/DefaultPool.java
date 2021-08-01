package org.strobe.gfx;

import java.util.Collections;
import java.util.Iterator;

public final class DefaultPool extends BindablePool<Bindable> {

    public DefaultPool() {
        super(new UnconditionalUnbinding());
    }


    @Override
    public void dispose(Graphics gfx, Bindable bindable) {
    }

    @Override
    public Iterator<Bindable> iterator() {
        return Collections.emptyIterator();
    }

    @Override
    public String toString() {
        return "---";
    }
}
