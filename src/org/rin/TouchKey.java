package org.rin;

import java.util.Iterator;
import java.util.LinkedList;
import java.lang.Math;

import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;




public class TouchKey implements OnTouchListener{

	public TouchKey()
	{
		Regions = new LinkedList<TouchRegion>();
	}
	
	LinkedList<TouchRegion> Regions;
	
	public void addRegion(TouchRegion aregion){
		Regions.add(aregion);
	}
	
	
	public boolean onTouch(View v, MotionEvent event) {


		/*Iterator<TouchRegion> it = Regions.iterator();
		while(it.hasNext())
		{
			TouchRegion current = it.next();
			current.detectTouch(event, event.getActionIndex());
		}*/
		
		
		for(int i = 0; i< event.getPointerCount();i++)
		{
			Iterator<TouchRegion> it = Regions.iterator();
			while(it.hasNext())
			{
				TouchRegion current = it.next();
				current.detectTouch(event, event.getActionIndex());
			}
		}
		return true;
	}
	public class TouchRect extends TouchRegion
	{
		event Event;
		public TouchRect(View aview, event aevent)
		{
			Event = aevent;
			view = aview;
		}

		@Override
		public void detectTouch(MotionEvent event, int index) {
			
			int width = view.getWidth();
			int height = view.getHeight();
			int x = view.getLeft();
			int y = view.getTop();
			
			if(event.getX(index) > x && event.getX(index) < x + width){
				if(event.getY(index) > y && event.getY(index) < y + height){
					Event.call(event, view, index);
				}
			}
		}
		
	}
	public class TouchCirc extends TouchRegion
	{

		public TouchCirc( View aview, event aevent)
		{
			Event = aevent;
			view = aview;
		}

		@Override
		public void detectTouch(MotionEvent event, int index) {
			
			int x = view.getLeft() + view.getWidth()/2;
			int y = view.getTop() + view.getHeight()/2;
			double radius = Math.sqrt(Math.pow(view.getWidth()/2,2) + Math.pow(view.getHeight()/2,2));
			
			if(Math.sqrt(Math.pow(x - event.getX(index),2) + Math.pow(y - event.getY(index),2))< radius){
				Event.call(event, view, index);
			}
		}
		
	}
	
	public abstract class TouchRegion
	{
		View view;
		event Event;
		public abstract void detectTouch(MotionEvent event, int index);
	}

	public interface event
	{
		public void call(MotionEvent event, View v, int index);
	}

}
