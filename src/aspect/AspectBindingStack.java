package aspect;

import java.util.HashMap;
import java.util.LinkedList;

/**
 * The BindingStack manages binding and unbinding of Bindables </br>
 * Mostly a package local class
 * @version 0.0
 * @author karl
 *
 */
public class AspectBindingStack {
	private HashMap<Class<? extends AspectBindable>, LinkedList<AspectBindable>> binding_stacks = new HashMap<>();
	/**
	 * Gets the currently bound Bindable at the binding spot
	 * @version 0.0
	 * @param bindable_class binding spot
	 * @return the Bindable currently bound to the binding spot
	 */
	public AspectBindable get(Class<? extends AspectBindable> bindable_class) {
		LinkedList<AspectBindable> binding_stack = binding_stacks.get(bindable_class);
		if(binding_stack == null || binding_stack.isEmpty())return null;
		return binding_stack.getFirst();
	}
	/**
	 * Checks if the bounding spot has something bound to it
	 * @version 0.0
	 * @param bindable_class
	 * @return true if there is something bound to the binding spot
	 */
	public boolean isBound(Class<? extends AspectBindable> bindable_class) {
		return binding_stacks.containsKey(bindable_class);
	}
	/**
	 * package local method that binds a bindable to the bindingStack </br>
	 * pushes a AspectPass into the renderqueue that binds the bindable at the right time
	 * @version 0.0
	 * @param render_queue 
	 * @param bindable
	 */
	void bind(AspectQueue render_queue, AspectBindable bindable) {
		LinkedList<AspectBindable> binding_stack = binding_stacks.computeIfAbsent(bindable.getClass(), c->new LinkedList<AspectBindable>());
		binding_stack.push(bindable);
		render_queue.add(new BindingPass(bindable));
	}
	/**
	 * package local method that unbinds a bindable from the bindingStack </br>
	 * pushes a AspectPass into the renderqueue that unbinds the bindable at the right time
	 * @version 0.0
	 * @param render_queue
	 * @param bindable
	 */
	void unbind(AspectQueue render_queue, AspectBindable bindable) {
		LinkedList<AspectBindable> binding_stack = binding_stacks.get(bindable.getClass());
		if(binding_stack == null)throw new IllegalStateException("can't unbind an unbind BindableClass");
		if(binding_stack.isEmpty()) throw new IllegalStateException("can't unbind an unbind Bindable");
		if(!binding_stack.pop().equals(bindable))throw new IllegalStateException("the Bindable to unbind is not currently bound");
		
		render_queue.add(new UnbindingPass(bindable));
		
		if(!binding_stack.isEmpty()) {
			render_queue.add(new BindingPass(binding_stack.getFirst()));
		}
	}
	/**
	 * clears the BindingStack
	 */
	void clear() {
		binding_stacks.clear();
	}
	/**
	 * A simple Pass that binds a Bindable
	 * @author karl
	 * @version 0.0
	 *
	 */
	private class BindingPass implements AspectPass{
		private final AspectBindable bindable;
		private BindingPass(AspectBindable bindable){
			this.bindable = bindable;
		}
		@Override
		public void execute() {
			bindable.bind();
		}
	}
	/**
	 * A simple Pass that unbinds a Bindable
	 * @author karl
	 * @version 0.0
	 *
	 */
	private class UnbindingPass implements AspectPass{
		private final AspectBindable bindable;
		private UnbindingPass(AspectBindable bindable){
			this.bindable = bindable;
		}
		@Override
		public void execute() {
			bindable.unbind();
		}
	}


}
