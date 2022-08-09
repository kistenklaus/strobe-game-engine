package aspect;

import java.util.ArrayList;
import java.util.LinkedList;

/**
 * An AspectLocation guilds to a set of nodes inside of the node graph </br>
 * The AspectLocation is definied by operations which when combined </br>
 * point to a spefific set of nodes.
 * @version 0.0
 * @author karl
 *
 */
public final class AspectLocation {
	
	private LinkedList<LocationOperation> operations = new LinkedList<>();
	private ArrayList<AspectNode> nodes = new ArrayList<>();
	private boolean recompute = true;
	private boolean computed = false;
	private AspectLocation() {
		
	}
	/**
	 * computes all nodes that match the defined pattern defined by the operators
	 * @version 0.0
	 * @param root
	 * @return
	 */
	ArrayList<AspectNode> compute(AspectNode root) {
		if(!computed || recompute) {
			//put all nodes into the arrayList
			nodes.clear();
			populateOptions(root);
			
			//compute all sub locations:
			for(LocationOperation operation : operations) {
				operation.compute(root);
			}
			
			//check if the node it self passes the criterial of the 
			
			for(LocationOperation operation : operations) {
				for(int i=nodes.size()-1;i>=0;i--) {
					if(!operation.compare(nodes.get(i))) {
						nodes.remove(i);
					}
				}
			}
			
			computed = true;
		}
		return nodes;
	}
	/**
	 * puts all nodes of the node graph inside of the location.nodes
	 * @version 0.0
	 * @param root
	 */
	private void populateOptions(AspectNode root) {
		nodes.add(root);
		for(AspectNode child : root.children()) {
			populateOptions(child);
		}
	}
	/**
	 * defines the location to be all nodes inside of the node graph
	 * @return this
	 */
	public final AspectLocation all() {
		return new AspectLocation();
	}public static final AspectLocation sall() {return new AspectLocation();}
	/**
	 * defines the location to only contain the root node
	 * @version 0.0
	 * @return this
	 */
	public final AspectLocation root() {
		return AspectLocation.sall().childOf(null);
	}public static final AspectLocation sroot() {return AspectLocation.sall().root();}
	@SafeVarargs
	/**
	 * defines the location to be contained in all location parameters and in the Node itself.
	 * @version 0.0
	 * @param locations
	 * @return this
	 */
	public final AspectLocation and(AspectLocation...locations) {
		operations.add(new AndOperation(locations));
		return this;
	}public static final AspectLocation sand(AspectLocation...locations) {return AspectLocation.sall().and(locations);}
	@SafeVarargs
	/**
	 * defines the location to be inside of one of the  spefified group(s)
	 * @version 0.0
	 * @param node_groups
	 * @return this
	 */
	public final AspectLocation in(String...node_groups) {
		operations.add(new InOperation(node_groups));
		return this;
	}public static final AspectLocation sin(String...node_groups) {return AspectLocation.sall().in(node_groups);}
	@SafeVarargs
	/**
	 * defines the location to be one of the specified nodes
	 * @version 0.0
	 * @param nodes
	 * @return this
	 */
	public final AspectLocation is(AspectNode...nodes) {
		operations.add(new IsOperation(nodes));
		return this;
	}public static final AspectLocation sis(AspectNode...nodes) {return AspectLocation.sall().is(nodes);}
	@SafeVarargs
	/**
	 * defines the location to be named as one of the spefified names
	 * @version 0.0
	 * @param names
	 * @return this
	 */
	public final AspectLocation is(String...names) {
		operations.add(new IsNamedOperation(names));
		return this;
	}public static final AspectLocation sis(String...names) {return AspectLocation.sall().is(names);}
	@SafeVarargs
	/**
	 * defines the location to be contained in one of the spefified locations
	 * @version 0.0
	 * @param locations
	 * @return
	 */
	public final AspectLocation or(AspectLocation...locations) {
		operations.add(new OrOperation(locations));
		return this;
	}public static final AspectLocation sor(AspectLocation...locations) {return AspectLocation.sall().or(locations);}
	@SafeVarargs
	/**
	 * defines the location to be not contained in one of the spefified locations
	 * @version 0.0
	 * @param locations
	 * @return this
	 */
	public final AspectLocation not(AspectLocation...locations) {
		operations.add(new NotOperation(locations));
		return this;
	}public static final AspectLocation snot(AspectLocation...locations) {return AspectLocation.sall().not(locations);}
	/**
	 * defines the location to be a parent of the spefified location
	 * @version 0.0
	 * @param location
	 * @return this
	 */
	public final AspectLocation parentOf(AspectLocation location) {
		operations.add(new ParentOfOperation(location));
		return this;
	}public static final AspectLocation sparentOf(AspectLocation location) {return AspectLocation.sall().parentOf(location);}
	/**
	 * defines the location to be a child of the spefified location
	 * @version 0.0
	 * @param location
	 * @return this
	 */
	public final AspectLocation childOf(AspectLocation location) {
		operations.add(new ChildOfOperation(location));
		return this;
	}public static final AspectLocation schildOf(AspectLocation location) {return AspectLocation.sall().childOf(location);}
	/**
	 * defines the location to be a sibling of the spefified location
	 * @version 0.0
	 * @param location
	 * @return this
	 */
	public final AspectLocation siblingOf(AspectLocation location) {
		operations.add(new SiblingOfOperation(location));
		return this;
	}public static final AspectLocation ssiblingOf(AspectLocation location) {return AspectLocation.sall().siblingOf(location);}
	/**
	 * @return an list of nodes that the location was computed to
	 */
	ArrayList<AspectNode> nodes(){
		return this.nodes;
	}
	
	public String toString() {
		StringBuilder str = new StringBuilder("[");
		for(AspectNode node : nodes) {
			str.append(node.getName()+",");
		}
		if(!nodes.isEmpty())str.deleteCharAt(str.length()-1);
		str.append("]");
		return str.toString();
	}
	
	
	/**
	 * LocationOperation are applied to an AspectLocation to define the Location
	 * @versio 0.0
	 * @author karl
	 *
	 */
	private abstract class LocationOperation{
		abstract boolean compare(AspectNode node);
		void compute(AspectNode node) {}
	}
	private final class IsOperation extends LocationOperation{
		private final AspectNode[] nodes;
		IsOperation(AspectNode...nodes){
			this.nodes = nodes;
		}
		@Override
		boolean compare(AspectNode node) {
			for(AspectNode comp_node : nodes) {
				if(comp_node == null)continue;
				if(comp_node.equals(node))return true;
			}
			return false;
		}
	}
	private final class IsNamedOperation extends LocationOperation{
		private final String[] names;
		IsNamedOperation(String...names){
			this.names = names;
		}
		@Override
		boolean compare(AspectNode node) {
			for(String name : names) {
				if(node.getName().equals(name))return true;
			}
			return false;
		}
	}
	private final class InOperation extends LocationOperation{
		private final String[] node_groups;
		InOperation(String...node_groups){
			this.node_groups = node_groups;
		}
		@Override
		boolean compare(AspectNode node) {
			for(String group : node_groups) {
				if(node.isGroupedAs(group))return true;
			}
			return false;
		}
	}
	
	private abstract class LocationsLocationOperation extends LocationOperation{
		protected final AspectLocation[] locations;
		protected LocationsLocationOperation(AspectLocation[] locations) {
			this.locations = locations;
		}
		void compute(AspectNode root) {
			for(AspectLocation location : locations)location.compute(root);
		}
	}
	
	private final class AndOperation extends LocationsLocationOperation{
		AndOperation(AspectLocation...locations){
			super(locations);
		}
		@Override
		boolean compare(AspectNode node) {
			for(AspectLocation location : locations) {
				if(location == null || !location.nodes().contains(node))return false;
			}
			return true;
		}
	}
	private final class OrOperation extends LocationsLocationOperation{
		OrOperation(AspectLocation...locations){
			super(locations);
		}
		@Override
		boolean compare(AspectNode node) {
			for(AspectLocation location : locations) {
				if(location == null)continue;
				for(AspectNode comp_node : location.nodes()) {
					if(!AspectLocation.this.nodes().contains(comp_node)){
						AspectLocation.this.nodes().add(comp_node);
					}
				}
			}
			return true;
		}
	}
	private final class NotOperation extends LocationsLocationOperation{
		NotOperation(AspectLocation...locations){
			super(locations);
		}
		@Override
		boolean compare(AspectNode node) {
			for(AspectLocation location : locations) {
				if(location == null)continue;
				if(location.nodes().contains(node))return false;
			}
			return true;
		}
	}
	private abstract class LocationLocationOperation extends LocationOperation{
		protected AspectLocation location;
		LocationLocationOperation(AspectLocation location){
			this.location = location;
		}
		void compute(AspectNode root) {
			if(location == null)return;
			location.compute(root);
		}
	}
	private final class ChildOfOperation extends LocationLocationOperation{
		ChildOfOperation(AspectLocation location){
			super(location);
		}
		@Override
		boolean compare(AspectNode node) {
			if(location == null) return node.parent() == null;
			for(AspectNode comp_node : location.nodes()) {
				if(node.parent() != null && node.parent().equals(comp_node))return true;
			}
			return false;
		}
	}
	private final class ParentOfOperation extends LocationLocationOperation{
		ParentOfOperation(AspectLocation location){
			super(location);
		}
		@Override
		boolean compare(AspectNode node) {
			if(location == null)return false;
			for(AspectNode child : node.children()) {
				for(AspectNode comp_node : location.nodes()) {
					if(child.equals(comp_node))return true;
				}
			}
			return false;
		}
		
	}
	private final class SiblingOfOperation extends LocationLocationOperation{
		SiblingOfOperation(AspectLocation location){
			super(location);
		}
		@Override
		boolean compare(AspectNode node) {
			if(location == null)return false;
			for(AspectNode comp_node : location.nodes()) {
				if(comp_node.parent() == null)return false;
				if(comp_node.parent().equals(node.parent()) && !comp_node.equals(node))return true;
			}
			return false;
		}
	}
	
}
