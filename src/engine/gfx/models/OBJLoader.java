package engine.gfx.models;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.joml.Vector2f;
import org.joml.Vector3f;

import engine.gfx.Loader;

public class OBJLoader {
	public static RawModel loadObjModel(String fileName, Loader loader) {
		FileReader fr = null;
		try {
			fr = new FileReader(new File(fileName));
		} catch (FileNotFoundException e) {
			System.err.println("Could not load the File: " + fileName);
			e.printStackTrace();
		}
		BufferedReader br = new BufferedReader(fr);
		String line;
		List<Vector3f> vertices = new ArrayList<>();
		List<Vector2f> textures = new ArrayList<>();
		List<Vector3f> normals = new ArrayList<>();
		List<Integer> indices = new ArrayList<>();
		float[] vArray = null;
		float[] tArray = null;
		float[] nArray = null;
		int[] iArray = null;
		String[] currLine = null;
		try {
			while((line = br.readLine()) != null) {
				currLine = line.split(" ");
//				System.out.println(line + "     line:       " + lineIndex);
				if(line.startsWith("v ")) {
					vertices.add(new Vector3f(Float.parseFloat(currLine[1]),Float.parseFloat(currLine[2]),Float.parseFloat(currLine[3])));
				}else if(line.startsWith("vt ")) {
					textures.add(new Vector2f(Float.parseFloat(currLine[1]),Float.parseFloat(currLine[2])));
				}else if(line.startsWith("vn ")) {
					normals.add(new Vector3f(Float.parseFloat(currLine[1]),Float.parseFloat(currLine[2]),Float.parseFloat(currLine[3])));
				}else if(line.startsWith("f ")) {
					tArray = new float[vertices.size() *2];;
					nArray = new float[vertices.size() *3];
					break;
				}
			}
			
			while((line = br.readLine()) != null) {
				if(!line.startsWith("f ")) {
					continue;
				}
				currLine = line.split(" ");
				String[] vertex1 = currLine[1].split("/");
				String[] vertex2 = currLine[2].split("/");
				String[] vertex3 = currLine[3].split("/");
				
				processVertex(vertex1, indices, textures, normals, tArray, nArray);
				processVertex(vertex2, indices, textures, normals, tArray, nArray);
				processVertex(vertex3, indices, textures, normals, tArray, nArray);
			}
			br.close();
		}catch (IOException e) {
			e.printStackTrace();
		}
		vArray = new float[vertices.size()*3];
		iArray = new int[indices.size()];
		int vertexPointer = 0;
		for(Vector3f vertex : vertices) {
			vArray[vertexPointer++] = vertex.x;
			vArray[vertexPointer++] = vertex.y;
			vArray[vertexPointer++] = vertex.z;
		}
		for(int i = 0; i<indices.size(); i++) {
			iArray[i] = indices.get(i);
		}
		return loader.loadToVAO(vArray, tArray, nArray, iArray);
	}
	
	private static void processVertex(String[] vertexData,
			List<Integer> indices, List<Vector2f> textures, List<Vector3f> normals,
			float[] tArray, float[] nArray) {
		int currVertexPointer = Integer.parseInt(vertexData[0]) -1;
		indices.add(currVertexPointer);
		Vector2f currTex = textures.get(Integer.parseInt(vertexData[1]) -1);
		tArray[currVertexPointer * 2] = currTex.x;
		tArray[currVertexPointer * 2+1] = 1-currTex.y;
		Vector3f currNorm = normals.get(Integer.parseInt(vertexData[2])-1);
		nArray[currVertexPointer*3] = currNorm.x;
		nArray[currVertexPointer*3+1] = currNorm.y;
		nArray[currVertexPointer*3+2] = currNorm.z;
	}
}
