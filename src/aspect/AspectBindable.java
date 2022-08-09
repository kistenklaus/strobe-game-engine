package aspect;

import strobe.StrobeException;

/**
 * All things bound to the GraphicsCard are Bindables
 * @version 0.0
 * @author karl
 */
public abstract class AspectBindable {
	/**
	 * creates the bindable
	 * @version 0.0
	 */
	private boolean created = false;
	public final void intern_create(){
		if(!created){
			if(!Thread.currentThread().getName().equals(Aspect.getThreadName())) throw new StrobeException("Illegal Thread");
			create();
			created = true;
		}
	}
	public abstract void create();
	/**
	 * executed on rendertime </br>
	 * binds the bindable
	 * @version 0.0
	 */
	public abstract void bind();
	/**
	 * executed on rendertime </br>
	 * unbinds the bindable
	 * @version 0.0
	 */
	public abstract void unbind();
	/**
	 * disposes the bindable from the VRAM
	 * @version 0.0
	 */
	public abstract void dispose();
}
