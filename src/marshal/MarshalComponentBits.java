package marshal;

import java.util.Arrays;

class MarshalComponentBits {

	long[] bits = {0};

	MarshalComponentBits () {
	}

	MarshalComponentBits (int nbits) {
		checkCapacity(nbits >>> 6);
	}

	boolean has (MarshalComponentType component_type) {
		return has(component_type.index());
	}
	
	boolean has(int component_index) {
		final int word = component_index >>> 6;
		if (word >= bits.length) return false;
		return (bits[word] & (1L << (component_index & 0x3F))) != 0L;
	}
	
	void put (MarshalComponentType component_type) {
		put(component_type.index());
	}
	
	void put(int component_index) {
		final int word = component_index >>> 6;
		checkCapacity(word);
		bits[word] |= 1L << (component_index & 0x3F);
	}
	
	private void checkCapacity (int len) {
		if (len >= bits.length) {
			long[] newBits = new long[len + 1];
			System.arraycopy(bits, 0, newBits, 0, bits.length);
			bits = newBits;
		}
	}

	void remove (MarshalComponentType component_type) {
		remove(component_type.index());
	}
	void remove(int component_index) {
		final int word = component_index >>> 6;
		if (word >= bits.length) return;
		bits[word] &= ~(1L << (component_index & 0x3F));
	}

	void clear () {
		Arrays.fill(bits, 0);
	}

	public int length () {
		long[] bits = this.bits;
		for (int word = bits.length - 1; word >= 0; --word) {
			long bitsAtWord = bits[word];
			if (bitsAtWord != 0) {
				for (int bit = 63; bit >= 0; --bit) {
					if ((bitsAtWord & (1L << (bit & 0x3F))) != 0L) {
						return (word << 6) + bit + 1;
					}
				}
			}
		}
		return 0;
	}

	boolean hasOne (MarshalComponentBits other) {
		long[] bits = this.bits;
		long[] otherBits = other.bits;
		for (int i = Math.min(bits.length, otherBits.length) - 1; i >= 0; i--) {
			if ((bits[i] & otherBits[i]) != 0) {
				return true;
			}
		}
		return false;
	}


	boolean containsAll (MarshalComponentBits other) {
		long[] bits = this.bits;
		long[] otherBits = other.bits;
		int otherBitsLength = otherBits.length;
		int bitsLength = bits.length;

		for (int i = bitsLength; i < otherBitsLength; i++) {
			if (otherBits[i] != 0) {
				return false;
			}
		}
		for (int i = Math.min(bitsLength, otherBitsLength) - 1; i >= 0; i--) {
			if ((bits[i] & otherBits[i]) != otherBits[i]) {
				return false;
			}
		}
		return true;
	}
	
	@Override
	public int hashCode() {
		final int word = length() >>> 6;
		int hash = 0;
		for (int i = 0; word >= i; i++) {
			hash = 127 * hash + (int)(bits[i] ^ (bits[i] >>> 32));
		}
		return hash;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		
		MarshalComponentBits other = (MarshalComponentBits) obj;
		long[] otherBits = other.bits;
		
		int commonWords = Math.min(bits.length, otherBits.length);
		for (int i = 0; commonWords > i; i++) {
			if (bits[i] != otherBits[i])
				return false;
		}
		
		if (bits.length == otherBits.length)
			return true;
		
		return length() == other.length();
	}
}