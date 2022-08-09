package strb.gfx;

import java.util.ArrayList;
import java.util.List;
import org.joml.Vector2f;
import strb.core.Console;

public class VAO {
	
	protected final int ID;
	protected int drawCount;
	
	public VAO(int id, int drawCount) {
		this.ID = id;
		this.drawCount = drawCount;
	}
	
	public int getID() {
		return ID;
	}

	public int getDrawCount() {
		return drawCount;
	}
	
	public static class Data{
		private static int dataIndex = 0;
		private final int dataID;
		private List<float[]> attributes;
		protected boolean[] dirty;
		private int[] indicies = null;
		private final VertexLayout VERTEX_LAYOUT;
		private int vertexCount;
		private boolean valid = false;
		private boolean[] dynamicAttribute;
		int cacheIndex = -1;
		
		public Data(String ...layout) {
			VERTEX_LAYOUT = new VertexLayout(layout);
			attributes = new ArrayList<float[]>(VERTEX_LAYOUT.getAttributeCount());
			dataID = dataIndex++;
			for(int i=0;i<VERTEX_LAYOUT.getAttributeCount();i++)attributes.add(null);
			dirty = new boolean[VERTEX_LAYOUT.getAttributeCount()];
			vertexCount = Integer.MAX_VALUE;
			dynamicAttribute = new boolean[attributes.size()];
		}
		
		public void setVertexAttribute(int attribloc, float[] attribute) {
			setVertexAttribute(attribloc, attribute, false);
		}
		
		public void setVertexAttribute(int attribloc, float[] attribute, boolean dynamic) {
			int vertexLengthInLayout = VERTEX_LAYOUT.getVertexAttribLengthOf(attribloc);
			if(attribute.length%vertexLengthInLayout==0)
				Console.error("attribute does not match the format or has an invalid size!", true);
			vertexCount = (int)Math.min(vertexCount, 
					Math.floor(attribute.length/vertexLengthInLayout));
			attributes.set(attribloc, attribute);
			dirty[attribloc] = true;
			dynamicAttribute[attribloc] = dynamic;
			valid = true;
		}
		
		public void updateVertexOfAttribute(int attribloc, int vertexIndex, float[] vertex) {
			float[] attribute = attributes.get(attribloc);
			int vertexOffset = vertexIndex*VERTEX_LAYOUT.getVertexAttribLengthOf(attribloc);
			for(int i=0;i<VERTEX_LAYOUT.getVertexAttribLengthOf(attribloc);i++) {
				attribute[i+vertexOffset] = vertex[i];
			}
			dirty[attribloc] = true;
		}
		
		public float[] getVertexAttribute(int attribloc) {
			return attributes.get(attribloc);
		}
		
		public void setIndicies(int[] indicies) {
			this.indicies = indicies;
		}
		
		public int[] getIndicies() {
			return indicies;
		}
		
		public boolean isValid() {
			//check if all attributes have the same vertexCount:
			for(int i=0;i<attributes.size();i++) {
				int attribVertexCount = attributes.get(i).length/VERTEX_LAYOUT.getVertexAttribLengthOf(i);
				if(attribVertexCount != vertexCount)return false;
			}
			return valid;
		}
		
		public int getVertexCount() {
			return vertexCount;
		}
		
		public int getVertexAttributeCount() {
			return attributes.size();
		}
		
		public int getVertexAttributeLength(int attribloc) {
			return VERTEX_LAYOUT.getVertexAttribLengthOf(attribloc);
		}
		
		public void setAttributeDynamic(int attribloc, boolean dynamic) {
			dynamicAttribute[attribloc] = dynamic;
		}
		public boolean isAttributeDynamic(int attribloc) {
			return dynamicAttribute[attribloc];
		}
		
		public boolean isDirtyAttribute(int attribloc) {
			return dirty[attribloc];
		}
		public void cleanAttribute(int attribloc) {
			dirty[attribloc] = true;
		}

		public static class Quad extends VAO.Data{
			private Vector2f size;
			private Vector2f position;
			private boolean centered;
			public Quad() {
				this(1.0f, 1.0f, 0.0f, 0.0f, false);
			}
			public Quad(float width, float height) {
				this(width,height, 0.0f, 0.0f, false);
			}
			public Quad(float width, float height, float xposition, float yposition, boolean centered) {
				super("vec3", "vec2");
				size = new Vector2f(width,height);
				position = new Vector2f(xposition, yposition);
				this.centered = centered;
				constructPositions();
				constructTextureCoords();
				constructIndicies();
			}
			private void constructPositions() {
				setAttributeDynamic(0, true);
				float w = size.x/2.0f;
				float h = size.y/2.0f;
				float xOffset = position.x + (centered?w:0);
				float yOffset = position.y + (centered?h:0);
				setVertexAttribute(0, new float[] {
						xOffset-h, yOffset-h, 0f,
						xOffset-h, yOffset+h, 0f,
						xOffset+h, yOffset+h, 0f,
						xOffset+h, yOffset-h, 0f
				});
			}
			private void constructTextureCoords() {
				setAttributeDynamic(0, false);
				setVertexAttribute(1, new float[] {
						1f, 0f,
		        		0f, 0f,
		        		0f, 1f,
		        		1f, 1f
				});
			}
			private void constructIndicies() {
				setIndicies(new int[] {
						2,1,0,
						3,0,2
				});
			}
			public void updateSize(float width, float height) {
				size = new Vector2f(width, height);
				constructPositions();
			}
			public void translate(float dx, float dy) {
				position.add(dx, dy);
				constructPositions();
			}
			public void setPosition(float x, float y) {
				position.set(x,y);
				constructPositions();
			}
		}
		
		public int hashCode() {
			return dataID;
		}
	}
	
	public static class VertexLayout {
		private int[] vertexAttribLengths;
		
		public VertexLayout(String ...stringlayout) {
			this.vertexAttribLengths = new int[stringlayout.length];
			for(int i=0;i<stringlayout.length;i++) {
				switch(stringlayout[i]) {
					case "vec4":
						vertexAttribLengths[i] = 4;
						break;
					case "vec3":
						vertexAttribLengths[i] = 3;
						break;
					case "vec2":
						vertexAttribLengths[i] = 2;
						break;
					default :
						Console.error("Illegal layout type:[" + stringlayout[i]+"]");
				}
			}
		}
		
		public int getAttributeCount() {
			return this.vertexAttribLengths.length;
		}
		
		public int getVertexAttribLengthOf(int attributeIndex) {
			return this.vertexAttribLengths[attributeIndex];
		}
		/**
		 * @deprecated
		 * @param other_vl
		 * @return
		 */
		protected boolean matches(VertexLayout other_vl) {
			for(int i=0;i<vertexAttribLengths.length;i++) {
				if(vertexAttribLengths[i] != other_vl.getVertexAttribLengthOf(i))return false;
			}
			return true;
		}
	}
	
	public int hashCode() {
		return ID;
	}
	
}
