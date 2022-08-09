package strb.ecs.components;

import strb.gfx.VAO;

public class Mesh {
	
	private final VAO.Data VAO_DATA;
	
	public Mesh(VAO.Data vaoData){
		this.VAO_DATA = vaoData;
	}
	
	public VAO.Data getVaoData(){
		return VAO_DATA;
	}
	
	
	
}
