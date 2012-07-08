package org.rin;

import android.app.Activity;
import android.app.Dialog;
import android.os.Handler;
import android.widget.TextView;

public class Notification {
	
	
	private static Dialog dialog;
	public static void showNotification(Activity act, String text)
	{
		showNotificationTimed(act, text, 2000);
	} 
	public static void showNotificationTimed(Activity act, String text, int timeMs)
	{
        //set up dialog
        dialog = new Dialog(act, android.R.style.Theme_Translucent_NoTitleBar_Fullscreen);
        dialog.setContentView(R.layout.notification);
        
        TextView nottext = (TextView) dialog.findViewById(R.id.nottext);
        nottext.setText(text);
        dialog.setOwnerActivity(act);
        dialog.show();
        
        dialog.setCancelable(true);
        
        Handler handler = new Handler(); 
        handler.postDelayed(new Runnable() { 
             public void run() { 
            	 dialog.dismiss();             } 
        }, timeMs); 
	}

}
