package engine.gfx.renderer;

import java.util.ArrayList;
import java.util.List;

import org.lwjgl.opengl.GL11;

import engine.gfx.entity.Entity2D;
import engine.gfx.models.Model2D;

public abstract class Renderer2D extends Renderer{
	private List<Entity2D> entities;
	public Renderer2D() {
		this.entities = new ArrayList<Entity2D>();
	}
	
	public void processEntity(Entity2D entity) {
		this.entities.add(entity);
	}
	
	public void enableRenderSetting() {
		GL11.glEnable(GL11.GL_BLEND);
		GL11.glBlendFunc(GL11.GL_SRC_ALPHA, GL11.GL_ONE_MINUS_SRC_ALPHA);
	}
	
	@Override
	public void render() {
		MasterRenderer.disableCulling();
		
		sortEntities();
		startShader();
		processScene();
		for(int i = 0; i < entities.size(); i++) {
			Entity2D entity = entities.get(i);
			bindModel(entity.getModel());
			prepareEntity(entity);
			GL11.glDrawElements(GL11.GL_TRIANGLES, entity.getModel().getRawModel2D().getVertexCount(), GL11.GL_UNSIGNED_INT, 0);
		}
		stopShader();
		MasterRenderer.enableCulling();
		entities.clear();
	}
	public void disableRenderSetting() {
		GL11.glDisable(GL11.GL_BLEND);
	}
	
	protected abstract void startShader();
	protected abstract void processScene();
	protected abstract void bindModel(Model2D model);
	protected abstract void prepareEntity(Entity2D entity);
	protected abstract void unbindModel();
	protected abstract void stopShader();
	
	private void sortEntities() {
		if(entities.size() != 0)
			quickSort(0,entities.size()-1);
	}
	
	private void quickSort(int lowerIndex, int higherIndex) {
		int i = lowerIndex;
        int j = higherIndex;
        // calculate pivot number, I am taking pivot as middle index number
        int pivot = entities.get(lowerIndex+(higherIndex-lowerIndex)/2).getLevel();
        // Divide into two arrays
        while (i <= j) {
            /**
             * In each iteration, we will identify a number from left side which 
             * is greater then the pivot value, and also we will identify a number 
             * from right side which is less then the pivot value. Once the search 
             * is done, then we exchange both numbers.
             */
            while (entities.get(i).getLevel() < pivot) {
                i++;
            }
            while (entities.get(j).getLevel() > pivot) {
                j--;
            }
            if (i <= j) {
                exchangeNumbers(i, j);
                //move index to next position on both sides
                i++;
                j--;
            }
        }
        // call quickSort() method recursively
        if (lowerIndex < j)
            quickSort(lowerIndex, j);
        if (i < higherIndex)
            quickSort(i, higherIndex);
	}
	private void exchangeNumbers(int i, int j) {
	      Entity2D temp = entities.get(i);
	      entities.set(i, entities.get(j));
	      entities.set(j, temp);
	}
}
