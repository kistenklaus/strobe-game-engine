package aspect;

import java.util.LinkedList;

/**
 * The AspectContext is the container for all nodes and</br>
 * also provieds a unique Thread for rendering the queue
 * @version 0.0
 * @author karl
 *
 */
public final class AspectContext implements Runnable {
	private final AspectNode root;
	private AspectQueue render_queue = new AspectQueue();
	private AspectBindingStack binding_stack = new AspectBindingStack();
	private final LinkedList<AspectPass> scheduled_passes = new LinkedList<>();
	private boolean running = false;
	private boolean render_queue_has_changed = false;
	private final LinkedList<AspectPass> root_init_passes = new LinkedList<>();
	private final LinkedList<AspectPass> root_down_passes = new LinkedList<>();
	private final LinkedList<AspectPass> root_up_passes = new LinkedList<>();
	private final LinkedList<AspectPass> root_dispose_passes = new LinkedList<>();


	private final Thread contextThread;
	
	public AspectContext(){
		this.root = new AspectNode("ROOT");
		this.contextThread = new Thread(this, Aspect.getThreadName());
		this.root.setInitPass(()->root_init_passes.forEach(AspectPass::execute));
		this.root.setDownPass(()->root_down_passes.forEach(AspectPass::execute));
		this.root.setUpPass(()->root_up_passes.forEach(AspectPass::execute));
		this.root.setDisposePass(()->root_dispose_passes.forEach(AspectPass::execute));
	}
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
	 * adds a node to the node graph at the specified locations
	 * @param node
	 * @param location
	 * @throw AspectException when the location points to the NULL-Node because the ROOT-Node has to be the only child of the NULL-Node
	 */
	public synchronized void addNode(AspectNode node, AspectLocation location) {
		//compute location:
		if (location == null) throw new AspectException("Cant add to the NULL-Node");
		location.compute(root);
		for (AspectNode loc_node : location.nodes()) {
			loc_node.appendChild(node, location.getPriority());
		}
		if (!location.nodes().isEmpty()) render_queue_has_changed = true;
	}
	/**
	 * removes the nodes from the node graph if they match the location
	 * @version 0.0
	 * @param location
	 * @throws AspectException when the location points to the NULL-Node or ROOT-Node because they can't be removed
	 */
	public synchronized void removeNode(AspectLocation location) {
		if (location == null) throw new AspectException("Cant remove the NULL-Node");
		location.compute(root);
		for (AspectNode loc_node : location.nodes()) {
			if (loc_node.parent() == null) throw new AspectException("Cant remove the ROOT-Node");
			loc_node.parent().removeChild(loc_node);
		}
		if (!location.nodes().isEmpty()) render_queue_has_changed = true;
	}
	/**
	 * schedules the pass to be executed directly after the context is started
	 * @param context_start_pass
	 */
	public synchronized void onStart(AspectPass context_start_pass){
		this.root_init_passes.add(context_start_pass);
	}
	/**
	 * schedules the pass to be executed directly after the context is stoped
	 * @param context_stop_pass
	 */
	public synchronized void onStop(AspectPass context_stop_pass){
		this.root_dispose_passes.add(context_stop_pass);
	}
	/**
	 * schedules the pass to be executed before the render cycle
	 * @version 0.0
	 * @param context_pre_cycle_pass
	 */
	public synchronized void beforeRender(AspectPass context_pre_cycle_pass){
		this.root_down_passes.add(context_pre_cycle_pass);
	}
	/**
	 * schedules the pass to be executed after render cycle
	 * @version 0.0
	 * @param context_post_cycle_pass
	 */
	public synchronized void afterRender(AspectPass context_post_cycle_pass){
		this.root_up_passes.add(context_post_cycle_pass);
	}
	/**
	 * creates a unique Thread for the Context and starts it.
	 */
	void start() {
		running = true;
		this.contextThread.start();
	}
	/**
	 * stops the context Thread
	 */
	void stop() {
		running = false;
		try {
			contextThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	@Override
	public void run() {
		root.setConnected_context(this);
		root.init();
		executeScheduledPasses();//should be empty
		render_queue_has_changed = true;
		generateRenderQueue();
		while(running) {
			render_queue.render();
			generateRenderQueue();
			executeScheduledPasses();
		}
		root.dispose();
	}
	/**
	 * processes the root node which creates the render queue
	 */
	private synchronized void generateRenderQueue() {
		if(!render_queue_has_changed)return;
		render_queue.clear();
		binding_stack.clear();
		root.process(render_queue, binding_stack);
		render_queue_has_changed = false;
	}
	/**
	 * executes all schedules passes
	 * @version 0.0
	 */
	private synchronized void executeScheduledPasses() {
		if (!scheduled_passes.isEmpty()) {
			for (AspectPass scheduled_pass : scheduled_passes) scheduled_pass.execute();
		}
		scheduled_passes.clear();
	}
	/**
	 * tells the context to construct a new render_queue after the next render_cycle
	 * @version 0.0
	 */
	synchronized void scheduleRenderQueueConstruction(){
		this.render_queue_has_changed = true;
	}
	/**
	 * tells the context to execute the pass after the next render_cycle
	 * @version 0.0
	 * @param pass
	 */
	synchronized  void schedulePass(AspectPass pass){
		this.scheduled_passes.add(pass);
	}

	/**
	 * @version 0.0
	 * @return true if the context is running
	 */
	public boolean isRunning() {
		return this.running;
	}
	public Thread getThread(){
		return this.contextThread;
	}
}
