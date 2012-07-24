package org.rin;



import java.io.FileOutputStream;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.RelativeLayout;


class RinView extends SurfaceView implements SurfaceHolder.Callback 
{    	
    private SurfaceHolder mSurfaceHolder;  
    private Bitmap mBitmap;
 
    private int SCR_WIDTH;
    private int SCR_HEIGHT;
    
    public final Bitmap.Config BITMAP_CONFIG = Bitmap.Config.RGB_565;

	private rin Rin;
	Context Context;
	
	public RinView(Context context, AttributeSet attrs) {
        super(context, attrs);

        Context = context;
       
        // register our interest in hearing about changes to our surface
        mSurfaceHolder = getHolder();
        mSurfaceHolder.addCallback(this);

        setFocusable(true); // make sure we get key events
    }
    public void screenshot(String filename)
    {
    	if(mBitmap != null && ! mBitmap.isRecycled())
    	
    	try 
    	{
    	       FileOutputStream out = new FileOutputStream(filename);
    	       mBitmap.compress(Bitmap.CompressFormat.PNG, 90, out);
    	       Notification.showNotificationTimed(this, getString(R.id.screenshot_taken)  ,1000);
    	} 
    	catch (Exception e) 
    	{
    	       Log.e("org.rin","rinview: exception in screenshot", e);
    	}


    }
    public  void  doDrawSingle() 
    {
       Canvas c=null;
        try 
        {
        	c = mSurfaceHolder.lockCanvas();

			//Bitmap b = Bitmap.createScaledBitmap(mBitmap, 480, 432, false);
			if(c != null)
				c.drawBitmap(mBitmap,0,0,null);
        }
        catch(Exception e)
        {
        	//log.d("org.rin", "exception when locking surface");
        } 
        finally {
            // do this in a finally so that if an exception is thrown
            // during the above, we don't leave the Surface in an
            // inconsistent state
            if (c != null) 
            {
                mSurfaceHolder.unlockCanvasAndPost(c);
            }
        }
	}
    
	public void setRin(rin arin)
	{
		Rin = arin;
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) 
	{
	}

    public void surfaceDestroyed(SurfaceHolder holder)
    {
    	if(Rin.getRinService().isRunning())
		{
			//log.d("org.rin", "RinView: surfaceDestroyed");
		
			Rin.getRinService().pause();
			Rin.getRinService().sleepNative();
			Rin.getRinService().disableDrawing();
			pgTerm();
			//Rin.getRinService().wakeNative();
		}
    }
	public void surfaceCreated(SurfaceHolder holder)
	{
    	//log.d("org.rin", "RinView:  surfaceCreated");
    	
    	WindowManager wm = (WindowManager) Context.getSystemService(android.content.Context.WINDOW_SERVICE);
    	Display display = wm.getDefaultDisplay();

    	float fitWidth =0;
    	float fitHeight=0;
    	
    	
    	float liWidth = display.getWidth();
    	float liHeight = display.getHeight();
    	
    	float gbWidth = 160;
    	float gbHeight = 144;
    	
    	float rWidth = 800;
    	float rHeight = 480;
    	
    	int marginTop=0,marginBottom=0,marginLeft=0,marginRight=0;
    	RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(0,0);  	
    	int orientation =0;
    	
    	//calculate dynamically the bitmap size to ensure a screen as large as possible
    	//differentiate between orientations and aspect ratios that are lower or higher than the optimised aspect ration of 0.6 (480/800)
    	//to achieve that, fit the bitmap to a imaginary cropped screen with a aspect ratio of (480/800) that uses in both orientations 
    	//as much space as possible. This way the other ui elements like buttons get enough space.
    	//However, it is still necessary to provide lower dp values vor smaller screens to avoid overlapping of views
    	if(getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE)
    	{
    		orientation = 2;
        	
    		if(liHeight/liWidth < rHeight/rWidth)
    		{
    			fitHeight = liHeight;
    			fitWidth = (gbWidth/gbHeight) * liHeight;
    		}
    		else
    		{
            	fitHeight = (rHeight/rWidth) * liWidth;
            	fitWidth = (gbWidth/gbHeight)*fitHeight;
    		}
    		params.addRule(RelativeLayout.CENTER_IN_PARENT,RelativeLayout.TRUE);
    	}
    	else if(getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT)
    	{
    		orientation = 1;
    		
        	if(liWidth/liHeight <  rHeight/rWidth)
        	{
        		fitWidth = liWidth;
        		fitHeight = (gbHeight/gbWidth)*fitWidth;
        	}
    		else
    		{
    			fitWidth = (rHeight/rWidth) * liHeight;
    			fitHeight = (gbHeight/gbWidth) *fitWidth;
    		}
        	params.addRule(RelativeLayout.CENTER_HORIZONTAL,RelativeLayout.TRUE);
    	}
    	SCR_WIDTH = (int) fitWidth;
    	SCR_HEIGHT = (int)fitHeight;
    	
    	params.width = SCR_WIDTH;
    	params.height = SCR_HEIGHT;

    	params.setMargins(marginLeft, marginTop, marginRight, marginBottom);

    	setLayoutParams(params);
    	
    	
    	mBitmap = Bitmap.createBitmap(SCR_WIDTH, SCR_HEIGHT, BITMAP_CONFIG);

    	
    	Rin.getRinService().sleepNative();
		pgInit(this, mBitmap,SCR_WIDTH, SCR_HEIGHT,orientation);
		Rin.getRinService().enableDrawing();
		Rin.getRinService().wakeNative();
		Rin.getRinService().unpause();
	
	}
	private native int pgInit(RinView view, Bitmap bitmap,int tr_x, int tr_y, int orientation);
	private native int pgTerm();

}