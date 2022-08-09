package aspect;
/**
 * A AspectCheck is only run on queue construction </br>
 * mainly used to check if a bindable is bound
 * @version 0.0
 * @author karl
 *
 */
public abstract class AspectCheck {

	public abstract boolean check(AspectBindingStack binding_stack);
	
}
