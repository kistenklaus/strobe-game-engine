package game;

import org.joml.Vector2f;
import org.joml.Vector3f;

import engine.Logic;
import engine.gfx.entity.Animation2D;
import engine.gfx.entity.Camera;
import engine.gfx.entity.Entity2D;
import engine.gfx.entity.Entity3D;
import engine.gfx.entity.Light3D;
import engine.gfx.models.Model2D;
import engine.gfx.models.Model3D;
import engine.gfx.models.RawModel2D;
import engine.gfx.models.RawModel3D;
import engine.gfx.textures.Material;
import engine.gfx.textures.Texture;
import engine.gfx.toolbox.Maths;
import engine.loading.Loader;
import game.terrrain.TerrainMap;
import game.terrrain.TerrainRenderer;
import game.terrrain.TerrainShader;
import game.terrrain.TerrainTexture;
import game.terrrain.TerrainTexurePack;

public class GameLogic extends Logic {
	public static final int WIDTH = 1280, HEIGHT = 960;
	private static final float FOV = 70, NEAR_PLANE = 0.1f, FAR_PLANE = 10000f;
	
	public GameLogic() {super(WIDTH, HEIGHT, "Rubik's Cube");}
	
	private EntityRenderer renderer;
	private TerrainRenderer terrainRenderer;
	private Game2DRenderer renderer2D;
	private Camera camera;
	
	private Entity3D entity3D_1, entity3D_2;
	private Entity2D entity2D_1;
	private Animation2D animation2D_1;
	private TerrainMap terrainMap;
	
	@Override
	public void init(Loader loader) {
		loader.createProjMatrix(WIDTH, HEIGHT, FOV, NEAR_PLANE, FAR_PLANE);
		
		this.camera = new Camera(new Vector3f(0,40,100));
		super.engine.setInput(new InputHandler());
		
		Light3D light = new Light3D(new Vector3f(20000,20000,20000), new Vector3f(1,0.77f,0.56f));
		
		EntityShader shader = new EntityShader(loader.getProjMatrix());
		this.renderer = new EntityRenderer(shader, light, camera);
		super.engine.addRenderer(renderer);
		
		TerrainShader terrainShader = new TerrainShader(loader.getProjMatrix());
		this.terrainRenderer = new TerrainRenderer(terrainShader, light, camera);
		this.engine.addRenderer(terrainRenderer);
		
		GameShader2D shader2D = new GameShader2D(Maths.create2DWindowMatrix(WIDTH, HEIGHT));
		this.renderer2D = new Game2DRenderer(shader2D);
		super.engine.addRenderer(renderer2D);
		
		Material entityMaterial1 = new Material(loader.loadTextureID("./res/stallTexture.png"));
		entityMaterial1.setShineDamper(10f);
		entityMaterial1.setReflectivity(3f);
		Material entityMaterial2 = new Material(loader.loadTextureID("./res/stonebrick_wall.png"));
		entityMaterial2.setShineDamper(10f);
		entityMaterial2.setReflectivity(0.5f);
		RawModel3D rawModel1 = loader.loadOBJModel("./res/dragon.obj");
		RawModel3D rawModel2 = loader.loadOBJModel("./res/stall.obj");
		
		this.entity3D_1 = new Entity3D(new Model3D(rawModel1, entityMaterial2), new Vector3f(0,20,-20), 5, 0, 0, 1);
		this.entity3D_2 = new Entity3D(new Model3D(rawModel2, entityMaterial1), new Vector3f(20,20,-40), 5, 0, 0, 1);
		
		
		Model2D model2d = new Model2D(RawModel2D.QUAD(200, 200, loader), new Texture(loader.loadTextureID("./res/woodenbox.jpg")));
		this.entity2D_1 = new Entity2D(model2d, new Vector2f(100, 100), 0, 0, 1);
		
		this.animation2D_1 = new Animation2D("./res/animation/", new Vector2f(200, 200), 1, 300, 300, 0, 1, 1f, super.engine.getClock(), loader);
		
		TerrainTexurePack texturepack = new TerrainTexurePack(
				new TerrainTexture(loader.loadTextureID("./res/grass.png")),
				new TerrainTexture(loader.loadTextureID("./res/dirt.png")),
				new TerrainTexture(loader.loadTextureID("./res/flowers.png")),
				new TerrainTexture(loader.loadTextureID("./res/path.png")));
		this.terrainMap = new TerrainMap(-2, -2, 4, 4, loader, texturepack, new TerrainTexture(loader.loadTextureID("./res/blendMap.png")), "./res/heightmap.png");
	}

	@Override
	public void repaint() {
		this.animation2D_1.update();
		entity3D_1.rotateY(1);
		entity3D_2.rotateY(-0.8f);
		this.renderer.processEntity(entity3D_1);
		this.renderer.processEntity(entity3D_2);
		this.renderer2D.processEntity(entity2D_1);
		this.renderer2D.processEntity(animation2D_1);
		this.terrainRenderer.processTerrain(terrainMap.getTerrains());
	}

	@Override
	public void terminate() {
		
	}
}
