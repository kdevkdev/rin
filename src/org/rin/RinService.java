package org.rin;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import org.rin.RinService;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;

public class RinService extends Service 
{
	public static final int MODE_GB = 1;
	public static final int MODE_SGB = 2;
	public static final int MODE_GBC = 3;

	private Order order;
	private String rinPath;
	private String savePath;
	private String romName;
	private String romPath;
	private String romSaveStatePath;
	private boolean mute;

	public String getRomSaveStatePath() 
	{
		return romSaveStatePath;
	}

	private RinAudio audio;
	private RinThread rinThread;
	private boolean mRunning;
	
	public boolean isRunning() 
	{
		return mRunning;
	}
	private void startup()
	{		
		//log.d("org.rin", "rinservice startup()");


		//detect with the help of libreloadcheck on what arch we are and if arm, if the cpu supports neon.
		//if thats the case load the librin-neon.so instead of librin.so
		//detect with the help of libreloadcheck on what arch we are and if arm, if the cpu supports neon.
		//if thats the case load the librin-neon.so instead of librin.so
        System.loadLibrary("preloadcheck");
        
        if(detectNeon())
        {
        	System.loadLibrary("rin-neon");
        }
        else
        {
        	System.loadLibrary("rin");
        }
        
        order = new Order();
        
        rinPath = getRinPath();
        savePath = getSavePath();
		new File(savePath).mkdirs();
		
    	init();     
    	audio = getAudio();
    	rinThread = getRinThread();
    	rinThread.setRinPath(rinPath);
    	disableDrawing();
    	rinThread.start();
    	mute = false;
    	
	}
    private void terminate()
    {
		saveConfig();
    	pause();
    	audio.stopAudio();
    	audio.terminateAudio();
		rinThread.stopGame();
		mRunning = false;
		//log.d("org.rin", "rinservice terminated");
	}
	
	@Override
	public void onCreate()
	{
		//log.d("org.rin","rinservice onCreate()");

		if(!mRunning)
		{
			startup();
	    	mRunning = true;
		}
		
	}
	public static String getRinPath()
	{
        return  Environment.getExternalStorageDirectory().getAbsolutePath() + "/rin/";
	}
	public static String getSavePath()
	{
		return  getRinPath() + "save/";
	}
	public static String constructRomSavePath(String romName)
	{
		return getSavePath() +  "sav." + romName;
	}
	@Override
	public void onDestroy() 
	{
		//log.d("org.rin","rinservice: onDestroy()");
		terminate();
	}
	
	@Override
	public void onStart(Intent intent, int startid) 
	{
		//service started
		//log.d("org.rin","rin service onStartCalled");
	}
	@Override
	public int onStartCommand(Intent intent,int flags, int startID)
	{
		//log.d("org.rin","rinservice: onStartCommand()");
		return START_STICKY;
	}
	
	@Override
	public IBinder onBind(Intent intent) 
	{
		//log.d("org.rin","rin service bound");
		return mBinder;
	}
	@Override
	public boolean onUnbind(Intent intent) 
	{
		return false;
	}
	
	
	/***************************************************/
	public void pauseSleep()
	{
		pause();
    	sleepNative();
	}
	public void wakeUnpause()
	{
		wakeNative();
		unpause();
	}
    public void pause()
    {
    	//log.d("org.rin","rinservice: pause()");
    	audio.stopAudio();
    	pauseNative();
    }
    public void unpause()
    {
    	//log.d("org.rin","rinservice: unpause()");
        unpauseNative();
        
        if(!mute)
        	audio.startAudio();
    }
    public void mute()
    {
    	//log.d("org.rin","rinservice: mute()");
    	audio.stopAudio();
    	disableWav();
    	mute = true;
    }
    public void unmute()
    {
    	//log.d("org.rin","rinservice: unmute()");
    	audio.startAudio();
    	enableWav();
    	mute = false;
    }
    /*****************************************************/
    public void changeMute()
    {
    	//log.d("org.rin","rinservice: changemute()");
    	if(mute) 
    		unmute();
    	
    	else if(!mute) 
    		mute();
    }
    public boolean getMute()
    {
    	return mute;
    }

    public void startRewind()
    {
    	boolean temp = mute;
    	mute();
    	mute = temp;
    	order.intorderBlocking(Order.O_REWIND, 1);
    }
    public void stopRewind()
    {
    	order.intorderBlocking(Order.O_REWIND, 0);
    	if(!mute)
    		unmute();
    }
    public void reset()
    {
    	if(romPath != null && !romPath.equals("") && romGetLoaded())
    	{
    		loadConfig();
    		order.stringorderBlocking(Order.O_LOAD_ROM,romPath);
    	}
    }
    public int  loadRom(String path)
    {
		saveConfig();
		
		romPath = path;
		romName = romPath.substring(romPath.lastIndexOf("/")+1);
		
		if(romName.startsWith("/"))
		{
				romName = romName.substring(1);
		}
		
		if(getAutoCopyRom())
		{
			String dir = path.substring(0, path.lastIndexOf("/")+1);
			
			if(!dir.equals(rinPath))
			{
				copyFile(path,rinPath + romName);
			}
			romPath = rinPath + romName;
		}
		
		romSaveStatePath = constructRomSavePath(romName);
		
		new File(romSaveStatePath).mkdir();

		loadConfig();
		setAutosavePath(romSaveStatePath + "/autosave.stat.gz");
		setSramPath(romSaveStatePath + "/sram.gz");
		order.stringorderBlocking(Order.O_LOAD_ROM,romPath);

		return 1;
    }
    
    public int loadState(String path)
    {

		order.stringorderBlocking(Order.O_LOAD_STATE,path);
		loadConfig();
		return 1;
    }
    public int saveState(String path)
    {
		saveConfig();
		return order.stringorderBlocking(Order.O_SAVE_STATE,path);
    }
    
    public void takeScreenShot()
    {
    	//order.stringorderBlocking(Order.O_TAKESCREENSHOT, romSaveStatePath +"/screen"+ DateUtils.now() + ".png");
    }
    public void saveConfig()
    {
    	 order.stringorderBlocking(Order.O_SAVE_CONFIG,romSaveStatePath + "/.config");
    	 order.stringorderBlocking(Order.O_SAVE_CONFIG_GLOBAL, rinPath + ".config_global");
    }
    public void loadConfig()
    {
    	 order.stringorderBlocking(Order.O_LOAD_CONFIG,romSaveStatePath + "/.config");
    	 order.stringorderBlocking(Order.O_LOAD_CONFIG_GLOBAL, rinPath + ".config_global");
    }
	
	
    private native boolean detectNeon();

	public native void setPad(int new_state);
	
	public native int pauseNative();
	public native int unpauseNative();
	
	private native void enableWav();
	private native void disableWav();
	
	public native int sleepNative();
	public native int wakeNative();
	
	public native void enableDrawing();
	public native void disableDrawing();
	
	private native RinAudio getAudio(); 
	private native RinThread getRinThread();
    private native int init();
    
    public native void toogleTurbo();
    public native boolean getTurbo();
    
    public native void toogleShowFPS();
    public native boolean getShowFPS();
    
    public native boolean romGetLoaded();
    public native int romGetType();
    public native void setAutosavePath(String  path);
    public native void setSramPath(String path);
    
    public native void toogleAutoCopyRom();
    public native boolean getAutoCopyRom();
    
    public class LocalBinder extends Binder 
    {
        RinService getService() {
            return RinService.this;
        }
    }
    private final IBinder mBinder = new LocalBinder();

   private void copyFile(String source, String dest)
   {
		File src = new File(source);
		File cpy = new File(dest);
		
		
		FileInputStream in;
		FileOutputStream out;
		
	    try 
	    {
			in = new FileInputStream(src);
			out= new FileOutputStream(cpy);
			
		    byte[] buffer = new byte[2048];
		    
		    //copy the file content in bytes 
		    int length;
		    while ((length = in.read(buffer)) > 0)
		    {

		    	out.write(buffer, 0, length);

		    }
    	    in.close();
    	    out.close();
		} 
	    catch (FileNotFoundException e) 
		{
	    	Log.e("org.rin","file not found exception in copyFile", e);
		} catch (IOException e) 
		{
			Log.e("org.rin","file in/out exception in copyFile", e);
		}
	    

   }
}

