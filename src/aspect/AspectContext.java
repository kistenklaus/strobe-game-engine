package aspect;

/**
 * The AspectContext is the container for all nodes and</br>
 * also provieds a unique Thread for rendering the queue
 * @version 0.0
 * @author karl
 *
 */
public abstract class AspectContext implements Runnable {
	private AspectNode root = new AspectNode("ROOT");
	private AspectQueue render_queue = new AspectQueue();
	private AspectBindingStack binding_stack = new AspectBindingStack();
	private boolean running = false;
	private boolean render_queue_has_changed = false;
	
	public AspectContext(){}
	/**
	 * overrideable </br>
	 * its called when the Context is started
	 */
	public abstract void init();
	/**
	 * overrideable </br>
	 * called before the render queue is rendered
	 */
	public void pre_render() {}
	/**
	 * overrideable </br>
	 * called after the render queue is rendered
	 * @version 0.0  
	 */
	public void post_render() {}
	/**
	 * creates a new node with the name "unnamed"
	 * @version 0.0
	 * @return AspectNode
	 */
	public AspectNode createNode() {
		return createNode("unnamed");
	}
	/**
	 * creates a new node
	 * @param name
	 * @version 0.0
	 * @return AspectNode
	 */
	public AspectNode createNode(String name) {
		return new AspectNode(name);
	}
	/**
	 * 
	 * @param node
	 * @param location
	 */
	public synchronized void addNode(AspectNode node, AspectLocation location) {
		//compute location:
		if(location == null)throw new IllegalArgumentException("Cant add to the NULL-Node");
		location.compute(root);
		for(AspectNode loc_node : location.nodes()) {
			loc_node.appendChild(node);
		}
		if(!location.nodes().isEmpty()) render_queue_has_changed = true;
	}
	
	public synchronized void removeNode(AspectLocation location) {
		if(location == null)throw new IllegalArgumentException("Cant remove the NULL-Node");
		location.compute(root);
		for(AspectNode loc_node : location.nodes()) {
			if(loc_node.parent() == null)throw new IllegalArgumentException("Cant remove the ROOT-Node");
			loc_node.parent().removeChild(loc_node);
		}
		if(!location.nodes().isEmpty()) render_queue_has_changed = true;
	}
	
	/**
	 * removes all nodes from the context
	 */
	public void cleanUp() {}
	/**
	 * creates a unique Thread for the Context and starts it.
	 */
	void start() {
		Thread context_thread = new Thread(this);
		running = true;
		context_thread.start();
	}
	/**
	 * stops the context Thread
	 */
	void stop() {
		running = false;
	}
	
	@Override
	public void run() {
		init();
		while(running) {
			pre_render();
			render_queue.render();
			
			post_render();
			if(render_queue_has_changed) {
				generateRenderQueue();
			}
		}
		cleanUp();
	}
	/**
	 * processes the root node which creates the render queue
	 */
	private synchronized void generateRenderQueue() {
		render_queue.clear();
		binding_stack.clear();
		root.process(render_queue, binding_stack);
		render_queue_has_changed = false;
	}
	
	
}
