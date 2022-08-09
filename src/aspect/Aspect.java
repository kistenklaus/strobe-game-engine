package aspect;
/**
 * @author karl
 * @version 0.0
 * 
 * Aspect is the static EntryPoint of the AspectLibary
 *
 */
public final class Aspect{
	
	private Aspect() {}
	
	private static AspectContext context = null;
	
	/**
	 * Sets the AspectContext
	 * @version 0.0
	 * @param context
	 * @return the set context
	 * @throws AspectException when the context is null
	 */
	public static AspectContext setContext(AspectContext context) {
		if(context == null)throw new AspectException("the Context can't be NULL");
		Aspect.context = context;
		return context;
	}
	/**
	 * starts the AspectContext
	 * @version 0.0
	 * @throws AspectException when the AspectContext is unset
	 */
	public static void start() {
		if(context == null)throw new AspectException("There is no Context definied");
		context.start();
	}
	/**
	 * stops the AspectContext
	 * @version 0.0
	 * @throws AspectException when the AspectContext is unset
	 */
	public static void stop() {
		if(context != null) {
			context.stop();
		}else {
			throw new AspectException("can't Context stop a Context when no Context is running");
		}
	}
	/**
	 * static refrence to add AspectNodes to the current AspectContext
	 * @param node the node to be added to the AspectContext
	 * @param location the location of all nodes the node should be added to
	 * @version 0.0
	 * @throws AspectException when the AspectContext is unset
	 */
	public static void addNode(AspectNode node, AspectLocation location) {
		if(context == null)throw new AspectException("AspectContext is not set");
		context.addNode(node, location);
	}
	/**
	 * static refrence to remove AspectNodes from the current AspectContext
	 * @version 0.0
	 * @param location
	 */
	public static void removeNode(AspectLocation location) {
		context.removeNode(location);
	}
	public static void removeNode(AspectNode node) {
		context.removeNode(AspectLocation.sis(node));
	}
}
