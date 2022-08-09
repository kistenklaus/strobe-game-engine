package aspect;

import java.util.ArrayList;
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
	private AspectCheck up_check = null, down_check = null;
	private AspectNode parent = null;
	private final String name;
	private ArrayList<AspectNode> children = new ArrayList<>();
	private HashSet<String> groups = new HashSet<>();
	
	public AspectNode(String name) {
		this.name = name;
	}
	/**
	 * processes the node graph hierarchy which creates the render_queue
	 * @version 0.0
	 * @param render_queue
	 * @param binding_stack
	 */
	void process(AspectQueue render_queue, AspectBindingStack binding_stack) {
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
	/**
	 * appends a child node to the node
	 * @version 0.0
	 * @param node
	 */
	public void appendChild(AspectNode node) {
		node.parent = this;
		children.add(node);
	}
	/**
	 * removes a child node from the node
	 * @version 0.0
	 * @param node
	 * @return true if the child was actually removed
	 */
	public boolean removeChild(AspectNode node) {
		node.parent = null;
		return children.remove(node);
	}
	/**
	 * removes all child nodes of the node
	 * @version 0.0
	 */
	public void removeAllChildren() {
		for(AspectNode child : children)child.parent = null;
		children.clear();
	}
	/**
	 * adds a bindable to the node
	 * @version 0.0
	 * @param bindable
	 */
	public void addBindable(AspectBindable bindable) {
		bindables.add(bindable);
	}
	/**
	 * removes a bindable from the node
	 * @version 0.0
	 * @param bindable
	 * @return
	 */
	public boolean removeBindable(AspectBindable bindable) {
		return bindables.remove(bindable);
	}
	/**
	 * removes all bindables from the node
	 * @version 0.0
	 */
	public void removeAllBindables() {
		bindables.clear();
	}
	/**
	 * sets the down_pass
	 * @version 0.0
	 * @param down_pass
	 */
	public void setDownPass(AspectPass down_pass) {
		this.down_pass = down_pass;
	}
	/**
	 * sets the up_pass
	 * @version 0.0
	 * @param up_pass
	 */
	public void setUpPass(AspectPass up_pass) {
		this.up_pass = up_pass;
	}
	/**
	 * sets the down_check
	 * @version 0.0
	 * @param down_check
	 */
	public void setDownCheck(AspectCheck down_check) {
		this.down_check = down_check;
	}
	/**
	 * sets the up_check
	 * @version
	 * @param up_check
	 */
	public void setUpCheck(AspectCheck up_check) {
		this.up_check = up_check;
	}
	/**
	 * groups the node as a spefified groups
	 * @version 0.0
	 * @param group_name
	 */
	public void groupAs(String group_name) {
		groups.add(group_name);
	}
	/**
	 * removes the node from a spefified group
	 * @version 0.0
	 * @param group_name
	 */
	public void ungroupAs(String group_name) {
		groups.remove(group_name);
	}
	/**
	 * @param group_name
	 * @version 0.0
	 * @return true if the node is in the spefified group
	 */
	boolean isGroupedAs(String group_name) {
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
