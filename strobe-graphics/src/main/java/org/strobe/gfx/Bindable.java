package org.strobe.gfx;

public abstract class Bindable<T extends BindablePool> {

    protected final T pool;

    public Bindable(Graphics gfx) {
        T pool;
        Pool poolAnno = this.getClass().getAnnotation(Pool.class);
        if (poolAnno == null || poolAnno.value() == null) {//default pool
            pool = (T) gfx.getPool(DefaultPool.class);
        } else {
            pool = (T) gfx.getPool(poolAnno.value());
        }
        this.pool = pool;
    }

    protected abstract void bind(Graphics gfx);

    protected abstract void unbind(Graphics gfx);

    protected abstract void dispose(Graphics gfx);


}
