package marshal;

abstract class MarshalEvent {
	abstract void poll(MarshalContext context);
	final static class ENTITY_ADDED extends MarshalEvent{
		private final MarshalEntity entity;
		ENTITY_ADDED(MarshalEntity entity) {
			this.entity = entity;
		}
		void poll(MarshalContext context) {
			context.dispatchEntityAdded(entity);
		}
	}
	
	final static class ENTITY_REMOVED extends MarshalEvent{
		private final MarshalEntity entity;
		ENTITY_REMOVED(MarshalEntity entity) {
			this.entity = entity;
		}
		void poll(MarshalContext context) {
			context.dispatchEntityRemoved(entity);
		}
	}
	
	final static class COMPONENT_ADDED extends MarshalEvent{
		private final MarshalComponent component;
		COMPONENT_ADDED(MarshalComponent component) {
			this.component = component;
		}
		void poll(MarshalContext context) {
			context.dispatchComponentAdded(component);
		}
		
	}
	final static class COMPONENT_REMOVED extends MarshalEvent{
		private final MarshalComponent component;
		COMPONENT_REMOVED(MarshalComponent component) {
			this.component = component;
		}
		void poll(MarshalContext context) {
			context.dispatchComponentRemoved(component);
		}
	}
	
}
