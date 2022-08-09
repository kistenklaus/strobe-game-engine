package aspect;

import java.util.ArrayList;
import java.util.List;

/**
 * The render_queue is a queue of passes that get executed in rendertime
 * @version 0.0
 * @author karl
 *
 */
final class AspectQueue {
	//ArrayList because iteration time is key for fps
	private List<AspectPass> render_queue = new ArrayList<>();
	/**
	 * executes all passes
	 * @version 0.0
	 */
	void render() {
		for(AspectPass pass : render_queue) {
			pass.execute();
		}
	}
	/**
	 * clears the queue
	 * @version 0.0
	 */
	void clear() {
		render_queue.clear();
	}
	/**
	 * adds a render pass to the queue
	 * @version 0.0
	 * @param pass
	 */
	void add(AspectPass pass) {
		render_queue.add(pass);
	}
	public String toString() {
		return render_queue.toString();
	}
	
}
