package org.rin;


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.util.zip.GZIPOutputStream;

import org.rin.AboutDialog.OnAboutDialogListener;
import org.rin.AutosaveDialog.OnAutosaveDialogListener;
import org.rin.TextDialog.OnTextDialogListener;
import org.rin.PaletteDialog.OnPaletteDialogListener;
import org.rin.ScreenDialog.OnScreenDialogListener;
import org.rin.SpeedDialog.OnSpeedDialogListener;
import org.rin.TouchKey;
import org.rin.TouchKey.event;
import org.rin.RinService;


import android.app.Activity;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.IBinder;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;

public class rin extends Activity 
{
	RinService rinService; 
	

	private static final int GET_PATH_ROM = 3;
	private static final int GET_PATH_STATE_LOAD = 4;
	private static final int GET_PATH_STATE_SAVE = 5;
	
	//application states flags
	private static final int APP_STATE_UNDEFINED = 0;
	private static final int APP_STATE_EMULATING = 1;
	private static final int APP_STATE_FILE_BROWSING_FOR_STATE = 2;
	private static final int APP_STATE_FILE_BROWSING_FOR_ROM = 3;
	private static final int APP_STATE_CHOOSING_PALETTE  = 4;
	private static final int APP_STATE_SCREEN_MENU = 5;
	private static final int APP_STATE_SPEED_MENU = 6;
	private static final int APP_STATE_AUTOSAVE_MENU = 7;
	private static final int APP_STATE_ABOUT_MENU = 8;
	private static final int APP_STATE_HELP_MENU = 9;
	private int appState; 

	
	private RinView rinView;
	private View touchView;
	private AudioManager audioManager;
	private ServiceConnection mServiceConnection;
	private Intent rinServiceIntent;
	private Menu rinMenu;
	private boolean mRinServiceBound;

	
	private int pad_state;
	
	private int last_first_dir;
	private int cur_first_dir;
	private int last_second_dir;
	private int cur_second_dir;

	//ugly
	private boolean stayPaused = false;
	private boolean  sc_bHandleActivityResult = false;
	private boolean  sc_bRomLoadArg = false;
	private int trequestCode; private int tresultCode; private Intent  tdata;
	private String tPath;
	
	/**********************************************************/
	public RinService getRinService() 
	{
		return rinService;
	}
	private void fileMigrater()
	{
		try
		{
			//this function does use some files to migrate from old to new versions
			//more specifically it does for now move *.srams savs into the corresponding save folder and copresses them
			
			File file = new File(RinService.getRinPath());
			FilenameFilter sramfilter = new FilenameFilter() 
			{
			    public boolean accept(File dir, String name) {
			       return (name.endsWith(".sram") || name.endsWith(".sram.gz"));
			    }
			};
			File[] childs = file.listFiles(sramfilter);
			
			for (File item:childs)
			{
				FileInputStream in = new FileInputStream(item);
				int size = (int) item.length();
				
				String name = item.getName().substring(item.getName().lastIndexOf("/") + 1);
				name = name.substring(0, name.lastIndexOf(".sram"));
				
				String romSavePath = RinService.constructRomSavePath(name);
				
				File test = new File(romSavePath);
				
				if(!test.exists())
				{
					test.mkdirs();
				}
				
				FileOutputStream ot = new FileOutputStream(romSavePath + "/" + "sram.gz");
				BufferedOutputStream sout = null;
				
				if(item.getName().endsWith(".sram"))
				{
					GZIPOutputStream out = new GZIPOutputStream(ot);
					sout = new BufferedOutputStream(out);
				}
				else if(item.getName().endsWith(".sram.gz"))
				{
					sout = new BufferedOutputStream(ot);
				}
				BufferedInputStream sin = new BufferedInputStream(in);
				byte [] buffer = new byte[size];
				sin.read(buffer, 0, size);
				sout.write(buffer, 0, size);
				sout.close();
				item.delete();
			}
		}
		catch(Exception e)
		{
			
		}
	}
	private void connectRinService()
	{
		Context con = getApplicationContext();
		rinServiceIntent = new Intent(this, RinService.class);
	   	 boolean res = con.bindService(rinServiceIntent,  new ServiceConnection (){

			public void onServiceConnected(ComponentName className, IBinder service) {
				
				mRinServiceBound = true;
				rinService  = ((RinService.LocalBinder)service).getService();
				//Log.d("org.rin", "onServiceConnected");
				
				initializeView();
				
				if(sc_bHandleActivityResult)
				{
					handleActivityResult(trequestCode, tresultCode, tdata);
					sc_bHandleActivityResult = false;
				}
				else if(sc_bRomLoadArg)
				{
					sc_bRomLoadArg = false;
					rinService.loadRom(tPath);
				}
			};

			public void onServiceDisconnected(ComponentName className) 
			{
				mRinServiceBound = false;
				//Log.d("org.rin", "onServiceDisconnected");
				
			}}, Context.BIND_DEBUG_UNBIND  );
	   	 
	   	 con.startService(rinServiceIntent);
	   	 if(res == false)
	   	 {
	   		 //Log.e("org.rin", "coulnt bind to service");
	   	 }
	}

	private void initializeView()
	{

        setContentView(R.layout.main);
    	rinView =(RinView) findViewById(R.id.rinview);
    	touchView = findViewById(R.id.touchview);
        rinView.setRin(this);
        
    	View cross = findViewById(R.id.ImageGBCross);
    	View ButtonA = findViewById(R.id.ImageButtonA);
    	View ButtonB = findViewById(R.id.ImageButtonB);
    	View ButtonSelect = findViewById(R.id.ImageButtonSelect);
    	View ButtonStart = findViewById(R.id.ImageButtonStart);
    	
    	TouchKey tKey = new TouchKey();
    	touchView.setOnTouchListener(tKey);
    	
    	
    	tKey.addRegion(tKey.new TouchCirc(cross,new event(){

			public void call(MotionEvent event, View v, int index) {
				double pi = java.lang.Math.PI;
				//set the center of the coordinate system to the middle of the crossimage
				float x = event.getX(index) - v.getWidth()/2 - v.getLeft();
				float y = event.getY(index) - v.getHeight()/2 - v.getTop();
				//compute angle
				double angle = java.lang.Math.atan2(y,x);
				
				//compute distance from touch point to the center of the crossimage
				double dist = java.lang.Math.hypot(x,y);
				double start = -pi + pi/8;
				
				//minimum distance from center to avoid unclear presses is 3/5 of the half crossimage width
				if(dist > (3/5) * (v.getWidth()/2))
				{
					//check in what quadrant the point lies
					if(angle > start){
						if(angle < start + pi/4){
							//Toast.makeText(v.getContext(), "up left", Toast.LENGTH_SHORT).show();
							cur_first_dir = 2;
							cur_second_dir = 3;
						}
						else if(angle < start + 2*(pi/4)){
							//Toast.makeText(v.getContext(), "up", Toast.LENGTH_SHORT).show();
							cur_first_dir = 2;
							cur_second_dir = 0;
						}
						else if(angle < start + 3*(pi/4)){
							//Toast.makeText(v.getContext(), "up right", Toast.LENGTH_SHORT).show();						
							cur_first_dir = 2;
							cur_second_dir = 4;
						}
						else if(angle < start + 4*(pi/4)){
							//Toast.makeText(v.getContext(), "right", Toast.LENGTH_SHORT).show();
							cur_first_dir = 4;
							cur_second_dir = 0;
						}
						else if(angle < start + 5*(pi/4)){
							//Toast.makeText(v.getContext(), "down right", Toast.LENGTH_SHORT).show();
							cur_first_dir = 1;
							cur_second_dir = 4;
						}
						else if(angle < start + 6*(pi/4)){	
							//Toast.makeText(v.getContext(), "down", Toast.LENGTH_SHORT).show();
							cur_first_dir = 1;
							cur_second_dir = 0;
						}
						else if(angle < start + 7*(pi/4)){	
							//Toast.makeText(v.getContext(), "down left", Toast.LENGTH_SHORT).show();
							cur_first_dir = 1;
							cur_second_dir = 3;
						}
						else if(angle < start + 8*(pi/4)){	
							//Toast.makeText(v.getContext(), "left", Toast.LENGTH_SHORT).show();
							cur_first_dir = 3;
							cur_second_dir = 0;
						}
					}
					else{
						//Toast.makeText(v.getContext(), "left", Toast.LENGTH_SHORT).show();
						cur_first_dir = 3;
						cur_second_dir = 0;
					}
					
					//directional bits start at 0x10 (5th bit) in pad_state
					//0x10 is down 		dir=1
					//0x20 is up		dir=2
					//0x40 is left		dir=3
					//0x80 is right		dir=4
					
					switch(event.getAction() & MotionEvent.ACTION_MASK)
					{
						case MotionEvent.ACTION_DOWN:
						case MotionEvent.ACTION_POINTER_DOWN:
							
							last_first_dir = cur_first_dir;
							last_second_dir = cur_second_dir;
							pad_state |= 1<<(cur_first_dir+3);
							
							if(cur_second_dir >0){
								pad_state |=1<<(cur_second_dir+3);
							}
							break;
						
						case MotionEvent.ACTION_UP:
						case MotionEvent.ACTION_POINTER_UP:
							
							last_first_dir = cur_first_dir;
							last_second_dir = cur_second_dir;
							pad_state &= ~(1<<(cur_first_dir+3));
							
							if(cur_second_dir >0){
								pad_state &=~(1<<(cur_second_dir+3));
							}
							break;
							
						
						case MotionEvent.ACTION_MOVE:
							pad_state &= ~(1<<(last_first_dir+3));
							pad_state |= 1<<(cur_first_dir+3);

						
							if(cur_second_dir >0){
								pad_state |=1<<(cur_second_dir+3);
							}
							if(last_second_dir >0){
								pad_state &=~(1<<(last_second_dir+3));
							}
							
							last_first_dir = cur_first_dir;
							last_second_dir = cur_second_dir;
							break;
					}
					rinService.setPad(pad_state);
				}
				
			}}));

    	tKey.addRegion(tKey.new TouchRect(ButtonA,new event(){
      	

			public void call(MotionEvent event, View v, int index) {
				
				switch(event.getAction() & MotionEvent.ACTION_MASK)
				{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN:
						v.setPressed(true);
						pad_state |= 1;
					break;
					
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP:
						v.setPressed(false);
						pad_state &= ~1;
					break;
				}
				rinService.setPad(pad_state);
			}

    	}));
    	
    	tKey.addRegion(tKey.new TouchRect(ButtonB,new event(){
          	

			public void call(MotionEvent event,View v, int index) {
				
				switch(event.getAction() & MotionEvent.ACTION_MASK)
				{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN:
						v.setPressed(true);
						pad_state |= 2;
					break;
					
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP:
						v.setPressed(false);
						pad_state &= ~2;
					break;
					
				}
				rinService.setPad(pad_state);

			}
    	}));
    	
      	tKey.addRegion(tKey.new TouchRect(ButtonStart,new event(){
        	  
			public void  call(MotionEvent event,View v, int index) {
				
				switch(event.getAction() & MotionEvent.ACTION_MASK)
				{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN:
						v.setPressed(true);
						pad_state |= 8;
					break;
					
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP:
						v.setPressed(false);
						pad_state &= ~8;
					break;
				}
				rinService.setPad(pad_state);
			}
    		
    	}));
 
     	tKey.addRegion(tKey.new TouchRect(ButtonSelect,new event(){
      		 
			public void call(MotionEvent event, View v, int index) {
				
				switch(event.getAction() & MotionEvent.ACTION_MASK)
				{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN:
						v.setPressed(true);
						pad_state |= 4;
					break;
					
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP:
						v.setPressed(false);
						pad_state &= ~4;
					break;
				}
				rinService.setPad(pad_state);
			}   	    		
    	}));
        
        pad_state = 0;
        
        audioManager = (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
       
		
	}
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle inState) 
    {
    	super.onCreate(inState);
        
        mRinServiceBound = false;
        
        new File(RinService.getRinPath()).mkdir();

        //Log.d("org.rin", "onCreate()");
        if(inState != null)
        {
        	appState = inState.getInt("APP_STATE");
        	
        }
        else
        {
        	appState = APP_STATE_UNDEFINED;
        }
        fileMigrater();
        
        if(appState == APP_STATE_UNDEFINED)
        {
        	File f = new File(RinService.getRinPath()+".config_global");
            if(!f.exists())
            {	
        		TextDialog helpDialog = new TextDialog(this,this, new OnTextDialogListener(){

    				public void onOk(int id) 
    				{
    					startup();
   				}}, R.string.help_menu_text);
        		helpDialog.show(this);
            }
            else
            {
            		startup();
            }
        }
        else
        {
        	if(rinService == null)
        	{
        		//start rinService
                connectRinService();
        	}
        }

    }
    private void startup()
    {
    	boolean loaddialog = true;
    	
    	
    	//lacy evaluation
    	if(getIntent() != null)
    	{
    			if(getIntent().getData() != null)
    			{
    				if(getIntent().getData().getEncodedPath() != null)
    				{
    		    		String filePath = getIntent().getData().getPath();
    		    		File test = new File(filePath);
    		    		
    		    		if(test.exists() && test.isFile())
    		    		{
    		    			String name = test.getPath();
    		    			
    		    			if(name.endsWith(".gb") || name.endsWith(".gbc") || name.endsWith(".sgb"))
    		    			{
    		    				sc_bRomLoadArg = true;
    		    				tPath = test.getAbsolutePath();
    		    				loaddialog = false;
    		    				
    		    	    		//start rinService
    		    	            connectRinService();
    		    			}
    		    		}
    				}
    			}
    	}
    	
    	
    	if(loaddialog)
    	{
    		//only do this if not yet emulating
    		appState = APP_STATE_FILE_BROWSING_FOR_ROM;
    		initializeFileDialog(FileLoadDialog.class, RinService.getRinPath(), GET_PATH_ROM,new String[] {"gb","gbc","zip","gz"}, null, true);
    	}
    }
    private void initializeFileDialog(Class <?> cls,String loc, int reqCode, String[] extArray, String root, boolean warn)
    {
        Intent FD = new Intent(this,cls);
        FD.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

        Bundle bundle = new Bundle();
        bundle.putCharSequenceArray("extFilter", extArray);
        bundle.putString("startLocation", loc);
        
        if(root != null)
        {
        	bundle.putString("root",root);
        }
        
        if(warn)
        {
        	bundle.putBoolean("WARN", true);
        }
        
        FD.putExtras(bundle);

        startActivityForResult(FD, reqCode);
    }
    private void handleActivityResult(int requestCode, int resultCode, Intent  data)
    {
    	if(resultCode == FileDialog.RESULT_EXIT)
    	{
    		exit();
    	}
    	else if(resultCode == RESULT_CANCELED)
    	{
    		//do nothing
    	}
    	else if(resultCode == RESULT_OK)
	   	{
	    	switch(requestCode)
	    	{
	    		
	    		case GET_PATH_ROM:
	    	    	
	                rinService.wakeNative();
	    			rinService.loadRom(data.getStringExtra("path"));
	    			appState = APP_STATE_EMULATING;
	    			stayPaused = false;
	 
	    		break;
	    		
	    		case GET_PATH_STATE_LOAD:
	    			
	    			int res;
	    			String path = data.getStringExtra("path");

	    			if(resultCode == RESULT_OK)
	    			{
		    			rinService.wakeNative();
	    				res = rinService.loadState(path);
	    				appState = APP_STATE_EMULATING;
	    				
	    				if(res > 0)
	    				{
	    					Notification.showNotificationTimed(this, "state load succeeded: " + res,700);
	    				}
	    				else
	    				{
	    					Notification.showNotificationTimed(this, "state load failed: " + res,1000);
	    				}
	    			}
	    			
	    	        stayPaused = false;
	    			
	    		break;
	    		
	    		case GET_PATH_STATE_SAVE:
	    			
	    			String path1 = data.getStringExtra("path");
	    			if(resultCode == RESULT_OK)
	    			{
		    			rinService.wakeNative();
	    				res = rinService.saveState(path1);
	    				
	    				if(res > 0)
	    				{
	    					Notification.showNotification(this, "state save succeeded: " + res);
	    				}
	    				else
	    				{
	    					Notification.showNotification(this, "state save failed: " + res);
	    				}
	    			}	
	    			
	                stayPaused = false;
	
	    		break;
	    	}
	    } 
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent  data){
    	
    	if(rinService == null)
    	{
    		//start rinService
            connectRinService();
            this.sc_bHandleActivityResult = true;
            this.tdata = data;
            this.trequestCode = requestCode;
            this.tresultCode = resultCode;
    	}
    	else
    	{
    		handleActivityResult(requestCode, resultCode, data);
    	}
    	   	
    }
    @Override
    protected void onSaveInstanceState(Bundle outState)
    {	
    	//Log.d("org.rin","rin: onSaveInstanceState");
    	outState.putInt("APP_STATE", appState);
    }
    
    @Override
    public void onDestroy()
    {
        	super.onDestroy();
        	//Log.d("org.rin", "rin: activity onDestroy");
    }

	/*@Override
    public void onPause()
    {
    	super.onPause();
    	rinService.pause();
    }
    
   @Override
    public void onResume()
    {
    	super.onResume();
    	//rinService.unpause();
    	
    }
    @Override
    public void onStart()
    {
    	super.onStart();
    	
    }
    @Override
    public void onRestart()
    {
    	super.onRestart();
    	
    }
    @Override
    public void onStop()
    {
    	super.onStop();
    	rinService.pause();
    }*/
        
    @Override
    public boolean onMenuOpened(int featureId, Menu menu)
    {
    	rinService.pauseSleep();
    	return true;
    }
    @Override 
    public void onBackPressed()
    {

    }
    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) 
    {
        if (keyCode == KeyEvent.KEYCODE_BACK) 
        {
        	
            return true;
        }
        return super.onKeyLongPress(keyCode, event);
    }

    
    @Override
    public void onOptionsMenuClosed(Menu menu) 
    {
    	if(!stayPaused)
    	{
    		rinService.wakeUnpause();
    		stayPaused = false;
    	}
    };
    @Override
    public boolean onCreateOptionsMenu(Menu menu) 
    {
    	
    	rinMenu = menu;
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.rin_menu, menu);
        setSoundMenuItemText();
        setTurboMenuItemText();
        setBooleanMenu(R.id.menu_show_fps, rinService.getShowFPS());
        setBooleanMenu(R.id.menu_autocopy_rom, rinService.getAutoCopyRom());
        
        return true;
    }
    @Override
    public boolean onPrepareOptionsMenu (Menu menu) 
    {

    	//menu.setGroupEnabled(R.id.menu_group_active_rom, true);
    	//change menu according to app state (rom type, rom laoded)
    	if(rinService.romGetLoaded())
    	{
    		menu.setGroupEnabled(R.id.menu_group_all, true);
    		menu.setGroupVisible(R.id.menu_group_all, true);
    		
        	switch(rinService.romGetType())
        	{
        		case RinService.MODE_GB:
        		break;

        		case RinService.MODE_SGB:
        		case RinService.MODE_GBC:
        			MenuItem item = menu.findItem(R.id.menu_gb_palette);
        			item.setEnabled(false);
        			item.setVisible(false);
        		break;
        		
        		default:
        			//menu.setGroupEnabled(R.id.menu_group_active_rom, false);
        		break;
        	}
    		
    	}
    	else
    	{
    		menu.setGroupEnabled(R.id.menu_group_all, false);
    		menu.setGroupVisible(R.id.menu_group_all, false);
    		
    		
    		menu.findItem(R.id.menu_about).setVisible(true);
    		menu.findItem(R.id.menu_exit).setVisible(true);
    		menu.findItem(R.id.menu_help).setVisible(true);
    		menu.findItem(R.id.menu_load_rom).setVisible(true);
    		
    		menu.findItem(R.id.menu_about).setEnabled(true);
    		menu.findItem(R.id.menu_exit).setEnabled(true);
    		menu.findItem(R.id.menu_help).setEnabled(true);
    		menu.findItem(R.id.menu_load_rom).setEnabled(true);
    	}
    	return super.onPrepareOptionsMenu(menu);
    }
    void setSoundMenuItemText()
    {
        MenuItem item = rinMenu.findItem(R.id.menu_sound);
        
        if(rinService.getMute() == true)	
        	item.setTitle(getString(R.string.sound) + ": " + getString(R.string.off));
        else
        	item.setTitle(getString(R.string.sound) + ": " + getString(R.string.on));
    }
    void setTurboMenuItemText()
    {
        MenuItem item = rinMenu.findItem(R.id.menu_turbo);
        
        if(rinService.getTurbo() == true)	
        {
        	item.setTitle(getString(R.string.turbo) + ": " + getString(R.string.on));
        }
        else
        {
        	item.setTitle(getString(R.string.turbo) + ": " + getString(R.string.off));
        }
    }
    void setBooleanMenu(int id, boolean state)
    {
        MenuItem item = rinMenu.findItem(id);
        
        if(state== true)	
        	item.setChecked(true);
        else
        	item.setChecked(false);
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
    	switch(keyCode)
    	{    		
    		case KeyEvent.KEYCODE_VOLUME_UP:
    				audioManager.adjustVolume(AudioManager.ADJUST_RAISE, AudioManager.FLAG_PLAY_SOUND);
    			return true;
    			
    		case KeyEvent.KEYCODE_VOLUME_DOWN:
    				audioManager.adjustVolume(AudioManager.ADJUST_LOWER, AudioManager.FLAG_PLAY_SOUND);
    			return true;
    			
    		case KeyEvent.KEYCODE_BACK:
    				rinService.startRewind();
    			return true;
    			
    		default:
    			return false;
    	}
    }
    void exit()
    {
    	if(rinService != null)
	    {
			rinService.wakeNative();
			stayPaused = true;
			Context con = getApplicationContext();
	
			try
			{
				con.unbindService(mServiceConnection);
			}
			catch(Exception IllegalArgumentException)
			{
				//Log.d("org.rin","service already unbound");
			}
	
			con.stopService(rinServiceIntent);
	    }
		finish();
		
		//Log.d("org.rin", "menu_exit");
    }
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {    	
    	switch(keyCode)
    	{    		

			case KeyEvent.KEYCODE_BACK:
					rinService.stopRewind();
					
				return true;
				
			default:
				return false;
    	}
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) 
    {
    	stayPaused = false;
        // Handle item selection
        switch (item.getItemId()) 
        {
    	case R.id.menu_help:
    		appState = APP_STATE_HELP_MENU;
    		stayPaused = true;
    		
    		TextDialog helpDialog = new TextDialog(this,this, new OnTextDialogListener(){

				public void onOk(int id) 
				{
		        	stayPaused = false;
		        	rinService.wakeUnpause();
					appState = APP_STATE_EMULATING;	
				}}, R.string.help_menu_text);
    		
    		helpDialog.show(this);
    	break;
        	case R.id.menu_about:
        		appState = APP_STATE_ABOUT_MENU;
        		stayPaused = true;
        		
        		AboutDialog aboutDialog = new AboutDialog(this,this, new OnAboutDialogListener(){

					public void onOk(int id) 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;	
					}});
        		aboutDialog.show(this);
        	break;
        	
        	case R.id.menu_autosave:
        		appState = APP_STATE_AUTOSAVE_MENU;
        		stayPaused = true;
        		AutosaveDialog autosaveDialog = new AutosaveDialog(this,this, new OnAutosaveDialogListener(){

					public void onCancel() 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;
					}

					public void onOk(int id) 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;	
					}});
        		autosaveDialog.show(this);
        	break;
        	
        	
	        case R.id.menu_show_fps:
	        	rinService.toogleShowFPS();
	        	setBooleanMenu(R.id.menu_show_fps,rinService.getShowFPS());
	        	appState= APP_STATE_EMULATING;
	        break;
	        
	        case R.id.menu_autocopy_rom:
	        	rinService.toogleAutoCopyRom();
	        	setBooleanMenu(R.id.menu_autocopy_rom,rinService.getAutoCopyRom());
	        	appState= APP_STATE_EMULATING;
	        break;
	        
	        case R.id.menu_reset:
	        	rinService.wakeNative();
	        	rinService.reset();
	        	appState= APP_STATE_EMULATING;
	        	stayPaused = false;
        	break;
        	
        	case R.id.menu_turbo:
        		rinService.toogleTurbo();
        		appState = APP_STATE_EMULATING; 
        		setTurboMenuItemText();
        		
        	break;
        	case R.id.menu_speed:
        		appState = APP_STATE_SPEED_MENU;
        		stayPaused = true;
        		SpeedDialog speedDialog = new SpeedDialog(this,this, new OnSpeedDialogListener(){

					public void onCancel() 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;
					}

					public void onOk(int id) 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;	
					}});
        		speedDialog.show(this);
        		
        	break;
        	case R.id.menu_screen:
        		
        		appState = APP_STATE_SCREEN_MENU;
        		stayPaused = true;
        		ScreenDialog screenDialog = new ScreenDialog(this,this, new OnScreenDialogListener(){

					public void onCancel() 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;
					}

					public void onOk(int id) 
					{
			        	stayPaused = false;
						rinService.wakeUnpause();
						appState = APP_STATE_EMULATING;	
					}});
        		screenDialog.show(this);
        		
        	break;
        
        	case R.id.menu_gb_palette:

        		appState = APP_STATE_CHOOSING_PALETTE;
        		stayPaused = true;
        		PaletteDialog pal_dialog = new PaletteDialog(this,this, new OnPaletteDialogListener(){

				public void onCancel() 
				{
					rinService.wakeUnpause();
		        	stayPaused = false;
		        	appState = APP_STATE_EMULATING;
				}

				public void onOk(int id) 
				{
		        	stayPaused = false;
					rinService.wakeUnpause();
					appState = APP_STATE_EMULATING;
				}});
			pal_dialog.show(this);
	        	
        	break;
	    	case R.id.menu_screenshot:
	    		rinService.wakeNative();
	    		rinView.screenshot(rinService.getRomSaveStatePath()+"/screen"+ DateUtils.now() + ".png");
	    		appState = APP_STATE_EMULATING; 
	    	break;
	    	        	
        	case R.id.menu_sound:
        		rinService.changeMute();
        		setSoundMenuItemText(); 
        		appState = APP_STATE_EMULATING; 
        	break;
        	
        	case R.id.menu_load_rom:
                appState = APP_STATE_FILE_BROWSING_FOR_ROM;
                stayPaused = true;
                rinService.wakeNative();
                rinService.saveConfig();
                rinService.sleepNative();
                initializeFileDialog(FileLoadDialog.class, RinService.getRinPath(), GET_PATH_ROM,new String[] {"gb","gbc","zip","gz"}, null, false);
        	break;
        	
        	
        	case R.id.menu_state_load:
        		
        		appState = APP_STATE_FILE_BROWSING_FOR_STATE;
        		if(rinService.romGetLoaded())
        		{
        			File tmp = null; 
        			stayPaused = true;

    				tmp = new File(rinService.getRomSaveStatePath());
	        		if(tmp.exists())
	        		{
	        			initializeFileDialog(FileLoadDialog.class,rinService.getRomSaveStatePath(), GET_PATH_STATE_LOAD, new String[] {"stat", "stat.gz"}, rinService.getRomSaveStatePath(), false);
	        		}
	        		else
	        		{
	        			Notification.showNotificationTimed(this, getString(R.string.no_save_path),2000);
	        		}
        		}
        		else
        		{
        			Notification.showNotification(this, getString(R.string.no_rom_loaded));
        		}
        		

                break;
        		
        	case R.id.menu_state_save:

        		appState = APP_STATE_FILE_BROWSING_FOR_STATE;
        		if(rinService.romGetLoaded())
        		{
                	new File( rinService.getRomSaveStatePath()).mkdir();
                	pad_state = 0;
                	stayPaused = true;
                	initializeFileDialog(FileSaveDialog.class,rinService.getRomSaveStatePath(), GET_PATH_STATE_SAVE, new String[] {"stat.gz"}, rinService.getRomSaveStatePath(), false);
        		}
        		else
        		{
        			Notification.showNotificationTimed(this, getString(R.string.no_rom_loaded),3500);
        		}
                
                break;
            
        	case R.id.menu_exit:
        		
        		exit();
                
	        default:  
	        	break;
        }

        return true;
    }    
}

