package strb.ecs;

import java.util.HashMap;
import strb.util.Bits;

public class Family {
	
	private static HashMap<String, Family> families = new HashMap<>();
	private static int familyIndex = 0;
	
	private final Bits bitmask;
	private final int index;
	
	private Family(Bits bitmask) {
		this.bitmask = bitmask;
		index = familyIndex++;
	}
	
	int getIndex() {
		return index;
	}
	
	boolean matches(Entity entity) {
		Bits entityComponentsBits = entity.getComponentBits();
		
		return entityComponentsBits.containsAll(bitmask);
	}
	
	@SafeVarargs
	public static Family all(Class<? extends Component>... component_classes) {
		Bits bitmask = ComponentType.getBitsFor(component_classes);
		Family family = requireFamily(bitmask);
		return family;
	}
	
	private static Family requireFamily(Bits bitmask) {
		String hash = getFamilyHash(bitmask);
		Family family = families.get(hash);
		if(family == null) {
			family = new Family(bitmask);
			families.put(hash, family);
		}
		return family;
	}
	private static String getFamilyHash(Bits bitmask) {
		StringBuilder str = new StringBuilder();
		if(!bitmask.isEmpty()) {
			str.append("bitmask:{").append(getBitsString(bitmask)).append("}");
		}
		return str.toString();
	}
	private static String getBitsString(Bits bits) {
		StringBuilder str = new StringBuilder();

		int numBits = bits.length();
		for (int i = 0; i < numBits; ++i) {
			str.append(bits.get(i) ? "1" : "0");
		}
		return str.toString();
	}
	
	@Override
	public int hashCode() {
		return index;
	}
	
}
