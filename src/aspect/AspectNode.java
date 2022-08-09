package aspect;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;

/**
 * An AspectNode is the main building block of Aspect. </br>
 * The Node is used to generate the render_queue </br>
 * the render_queue is created by adding passes to the render_queue in a spefied order </br>
 * <ls>
 * <li>adding a pass that binds all bindables</li>
 * <li>checking if the correct bindables are bound to push this node to the render_queue (only executed when constructing the render_queue)</li>
 * <li>adding a pass that executes on the down_cycle (down_pass)</li>
 * <li>adding all passes of the child nodes</li>
 * <li>checking if the correct bindables are bound at the up_cyle (up_check) (only executed when constructing the render_queue)</li>
 * <li>adding a pass that executes on the up_cycle (up_pass)</li>
 * <li>adding a pass that unbinds all bindables</li>
 * </ls>
 * @version 0.0
 * @author karl
 *
 */
public final class AspectNode {
	private ArrayList<AspectBindable> bindables = new ArrayList<>();
	private AspectPass up_pass = null, down_pass = null;
	private AspectPass init_pass = null, dispose_pass = null;
	private AspectCheck up_check = null, down_check = null;
	private AspectNode parent = null;
	private final String name;
	private final ArrayList<AspectNode> children = new ArrayList<>();
	private final ArrayList<Integer> priority = new ArrayList<>();
	private HashSet<String> groups = new HashSet<>();
	private AspectContext connected_context = null;
	
	AspectNode(String name) {
		this.name = name;
	}
	/**
	 * processes the node graph hierarchy which creates the render_queue
	 * @version 0.0
	 * @param render_queue
	 * @param binding_stack
	 */
	synchronized void process(AspectQueue render_queue, AspectBindingStack binding_stack) {
		for(AspectBindable bindable : bindables) {
			binding_stack.bind(render_queue, bindable);
		}
		
		if(down_check != null && !down_check.check(binding_stack)) throw new IllegalStateException("failed downCheck for RenderNode : " + this);
		
		if(down_pass != null) render_queue.add(down_pass);
		
		for(AspectNode child : children) {
			child.process(render_queue, binding_stack);
		}
		
		if(up_check != null && !up_check.check(binding_stack)) throw new IllegalStateException("failed upCheck for RenderNode : " + this);
		
		if(up_pass != null) render_queue.add(up_pass);
		
		for(AspectBindable bindable : bindables) {
			binding_stack.unbind(render_queue, bindable);
		}
	}
	void init(){
		for(AspectBindable bindable : bindables){
			bindable.intern_create();
		}
		if(init_pass != null)init_pass.execute();
		for(AspectNode child : children){
			child.connected_context = this.connected_context;
			child.init();
		}

	}
	void dispose(){
		for(AspectNode child : children){
			child.dispose();
		}
		for(AspectBindable bindable : bindables){
			bindable.dispose();
		}
		if(dispose_pass != null)dispose_pass.execute();
	}

	/**
	 * appends a child with priority 0
	 * @param node
	 */
	public synchronized void appendChild(AspectNode node){
		appendChild(node, 0);
	}
	/**
	 * appends a child node to the node with the specified priority
	 * @param node
	 * @param priority
	 */
	public synchronized void appendChild(AspectNode node, int priority) {
		node.parent = this;
		node.connected_context = this.connected_context;
		boolean added = false;
		for(int i=0;i<children.size();i++){
			if(this.priority.get(i)>priority){
				this.priority.add(i, priority);
				this.children.add(i, node);
				added = true;
				break;
			}
		}
		if(!added){
			this.priority.add(priority);
			children.add(node);
		}

		if(connected_context != null){
			connected_context.schedulePass(()->node.init());
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * removes a child node from the node
	 * @version 0.0
	 * @param node
	 * @return true if the child was actually removed
	 * @throws AspectException when there is an error removing the child (should not happen in normal cases)
	 */
	public synchronized boolean removeChild(AspectNode node) {
		if(!node.parent.equals(this))return false;
		node.parent = null;
		node.connected_context = null;
		boolean removed =  children.remove(node);
		if(!removed) throw new AspectException("(ASPECT CODE ERROR!!!) Something went wrong removing a childNode");
		if(connected_context != null){
			connected_context.schedulePass(()->node.dispose());
			connected_context.scheduleRenderQueueConstruction();
		}
		return true;
	}
	/**
	 * removes all child nodes of the node
	 * @version 0.0
	 */
	public synchronized void removeAllChildren() {
		for(AspectNode child : children)removeChild(child);
	}
	/**
	 * adds a bindable to the node
	 * @version 0.0
	 * @param bindable
	 */
	public synchronized void addBindable(AspectBindable bindable) {
		if(bindables.contains(bindable))return;
		bindables.add(bindable);
		if(connected_context != null){
			connected_context.schedulePass(()->bindable.intern_create());
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * removes a bindable from the node
	 * @version 0.0
	 * @param bindable
	 * @return
	 */
	public synchronized boolean removeBindable(AspectBindable bindable) {
		boolean removed = bindables.remove(bindable);
		if(removed && connected_context != null){
			connected_context.schedulePass(()->bindable.dispose());
			connected_context.scheduleRenderQueueConstruction();
		}
		return removed;
	}
	/**
	 * removes all bindables from the node
	 * @version 0.0
	 */
	public synchronized void removeAllBindables() {
		for(AspectBindable bindable : bindables)removeBindable(bindable);
	}
	/**
	 * sets the down_pass
	 * @version 0.0
	 * @param down_pass
	 */
	public synchronized void setDownPass(AspectPass down_pass) {
		this.down_pass = down_pass;
		if(!this.down_pass.equals(down_pass) && connected_context != null){
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * sets the up_pass
	 * @version 0.0
	 * @param up_pass
	 */
	public synchronized void setUpPass(AspectPass up_pass) {
		this.up_pass = up_pass;
		if(!this.up_pass.equals(up_pass) && connected_context != null){
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * sets the dispose_pass
	 * @version 0.0
	 * @param dispose_pass
	 */
	public synchronized void setDisposePass(AspectPass dispose_pass) {
		this.dispose_pass = dispose_pass;
	}

	/**
	 * sets the init_pass
	 * @version 0.0
	 * @param init_pass
	 * @throws AspectException when the node is connected to the node graph
	 */
	public synchronized void setInitPass(AspectPass init_pass){
		if(connected_context != null)throw new AspectException("init_pass can only be set while the node is not connected to the node graph");
		this.init_pass = init_pass;
	}
	/**
	 * sets the down_check
	 * @version 0.0
	 * @param down_check
	 */
	public synchronized void setDownCheck(AspectCheck down_check) {
		this.down_check = down_check;
		if(!this.down_check.equals(down_check) && connected_context != null){
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * sets the up_check
	 * @version
	 * @param up_check
	 */
	public synchronized void setUpCheck(AspectCheck up_check) {
		this.up_check = up_check;
		if(!this.up_check.equals(up_check) && connected_context != null){
			connected_context.scheduleRenderQueueConstruction();
		}
	}
	/**
	 * groups the node as a specified groups
	 * @version 0.0
	 * @param group_name
	 */
	public synchronized void groupAs(String group_name) {
		groups.add(group_name);
	}
	/**
	 * removes the node from a spefified group
	 * @version 0.0
	 * @param group_name
	 */
	public synchronized void ungroupAs(String group_name) {
		groups.remove(group_name);
	}
	/**
	 * @param group_name
	 * @version 0.0
	 * @return true if the node is in the spefified group
	 */
	synchronized boolean isGroupedAs(String group_name) {
		return groups.contains(group_name);
	}
	/**
	 * @return all child nodes of the node
	 * @version 0.0
	 */
	ArrayList<AspectNode> children(){
		return this.children;
	}
	/**
	 * @return the parent node of the node
	 * @version 0.0
	 */
	AspectNode parent() {
		return this.parent;
	}
	/**
	 * @return the name of the node
	 * @version 0.0
	 */
	public String getName() {
		return name;
	}
	void setConnected_context(AspectContext connected_context){
		this.connected_context = connected_context;
	}
	public String toString() {
		StringBuilder str = new StringBuilder("[");
		if(parent == null)str.append("NULL");
		else str.append(parent.name);
		str.append(" -> @"+name + "-> (");
		for(AspectNode child : children)str.append(child.name+",");
		if(!children.isEmpty())str.deleteCharAt(str.length()-1);
		str.append(")]");
		return str.toString();
	}
}
