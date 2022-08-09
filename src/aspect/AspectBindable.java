package aspect;

/**
 * All things bound to the GraphicsCard are Bindables
 * @version 0.0
 * @author karl
 */
public abstract class AspectBindable {
	
	/**
	 * executed on rendertime </br>
	 * binds the bindable
	 */
	public abstract void bind();
	/**
	 * executed on rendertime </br>
	 * unbinds the bindable
	 */
	public abstract void unbind();
	
}
