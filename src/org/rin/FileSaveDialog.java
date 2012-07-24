package org.rin;

import java.io.File;

import org.rin.TextDialog.OnTextDialogListener;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

class ButtonListener implements OnClickListener {

	FileSaveDialog SFD;
	
	ButtonListener(FileSaveDialog sfd)
	{
		SFD= sfd;
	}
	
	public void onClick(View v) {
		SFD.onSaveButtonClick(v);
	}
}

public class FileSaveDialog extends FileDialog
{
	
		private long touchDownTime =0;
		EditText  editText;
		Button	saveButton;
		ButtonListener saveButtonListener;
		
		//config options
		boolean autoAppendExt = true; //(always take the provided extension)
		
		public void setAutoAppendExt(boolean autoAppendExt)
		{
			this.autoAppendExt = autoAppendExt;
		}
		
		static final int DIALOG_FILE_EXISTS = 0;
		
		@Override
		public void onCreate(Bundle inState)
		{
				super.onCreate(inState); 
				setContentView(R.layout.filedialog_save);
				
				saveButtonListener = new ButtonListener(this);
				
            	editText = (EditText) findViewById(R.id.filename);
            	editText.setOnEditorActionListener(new OnEditorActionListener(){

					public boolean onEditorAction(TextView v, int actionId,
							KeyEvent event) {

						saveButtonListener.onClick(v);
						
						return true;
					}});
            	
            	if(inState != null)
            	{
            		editText.setText(inState.getCharSequence("SAVE_EDIT_TEXT"));
            	}
            	
            	saveButton = (Button) findViewById(R.id.save);
            	saveButton.setOnClickListener(saveButtonListener);
				init(inState);		
		}
	    @Override
	    public boolean onOptionsItemSelected(MenuItem item) 
	    {
	    	boolean ret;
	        // Handle item selection
	        switch (item.getItemId())
	        {
	        	case R.id.fdmenu_help:
	        		TextDialog helpDialog = new TextDialog(this,this, new OnTextDialogListener(){

	    				public void onOk(int id) 
	    				{
	    				}}, R.string.help_fdsavemenu_text);
	        		
	        		helpDialog.show(this);
	        		ret = true;
	        	break;
	        
	        	default:
	        		ret=  super.onOptionsItemSelected(item);
	        	break;
	        		
	        }
	        return ret;
	    }
		public void onSaveButtonClick(View v)
		{
			hideSoftKeyboard();
			//various checks
			boolean found = false;
			String name = editText.getText().toString();
			if(name.equals(""))
			{
				Notification.showNotification(this, getString(R.string.empty_filename));
				return;
			}

			
			for(String ext:Filter.getExtensions()){
				if(name.toLowerCase().endsWith(ext.toLowerCase()))
				{
					found = true;
				}
			}
			
			if(!found && autoAppendExt)
			{
				found = true;
				if(!Filter.getExtensions()[0].equals("*") && !Filter.getExtensions()[0].equals(""))
				{
					name = name + "." + Filter.getExtensions()[0];
				}
			}
			
			
			if(found)
			{
				File tmp = new File(currentDir, name);
				Bundle bundle = new Bundle();
				try
				{
					bundle.putString("filename",tmp.getCanonicalPath());
				}
				catch(Exception ex)
				{
					Log.e("savefiledialog", "getcanonical path failed in onSaveButtonClick()");
				}
				
				if(tmp.exists())
				{
					showDialog(DIALOG_FILE_EXISTS,bundle);
				}
				else
				{
					sendResult(bundle);
				}
			}
			else
			{
				Notification.showNotification(this,getString(R.string.invalid_save_file_extension));
			}
		}
		public void sendResult(Bundle bundle)
		{
			Intent tmp = new Intent();
			tmp.putExtra("path", bundle.getString("filename"));
			setResult(Activity.RESULT_OK, tmp);
			finish();
		}

		/**
		 * Called to process touch screen events. 
		 */
		@Override
		public boolean dispatchTouchEvent(MotionEvent ev) {
			switch (ev.getAction()){
	        case MotionEvent.ACTION_DOWN:
	            touchDownTime = SystemClock.elapsedRealtime();
	            break;
	
	        case MotionEvent.ACTION_UP:
	            //to avoid drag events
	            if (SystemClock.elapsedRealtime() - touchDownTime <= 150)
	            {             	
	            	if(!eventOverView(ev,editText) && !eventOverView(ev,saveButton))
	            	{
	            		hideSoftKeyboard();
	            	}
	            }
	            break;
	    }
	    return super.dispatchTouchEvent(ev);
	}
		
		
		private void hideSoftKeyboard()
		{
			InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
			imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
		}
		private boolean eventOverView(MotionEvent ev, View view)
		{
			if(view.getRight() > ev.getRawX() && view.getLeft()< ev.getRawX())
			{
				if(view.getTop() < ev.getRawY() && view.getBottom() > ev.getRawY())
				{
					return true;
				}
			}
			return false;
		}

		@Override
		public void onFileClick(int pos, FileAdapter list) {
			editText.setText(list.getItem(pos).getName());
		}
		
		@Override
		protected Dialog onCreateDialog(int id, Bundle bundle) {
		    Dialog dialog;
		    switch(id) {
		    case DIALOG_FILE_EXISTS:
		        
		    	AlertDialog.Builder builder = new AlertDialog.Builder(this);
		    	builder.setMessage(R.string.overwrite)
		    	       .setCancelable(false)
		    	       .setPositiveButton(R.string.yes, new DialogYesClickListener(this, bundle))
		    	       .setNegativeButton(R.string.no, new DialogNoClickListener(this));
		    	AlertDialog alert = builder.create();
		    	dialog = alert;
		    	
		    	
		        break;
		      
		     default:
		        dialog = null;
		    }
		    return dialog;
		}
	    @Override
	    protected void onSaveInstanceState(Bundle outState)
	    {	
	    	super.onSaveInstanceState(outState);
	    	outState.putCharSequence("SAVE_EDIT_TEXT", editText.getText().toString());
	    }
}
class DialogYesClickListener implements DialogInterface.OnClickListener
{
	FileSaveDialog SFD;
	Bundle Bundle;
	
	DialogYesClickListener(FileSaveDialog sfd, Bundle bundle)
	{
		SFD = sfd;
		Bundle = bundle;
	}
	
	public void onClick(DialogInterface dialog, int which) 
	{
		SFD.sendResult(Bundle);
	}
	
}
class DialogNoClickListener implements DialogInterface.OnClickListener
{
	FileSaveDialog SFD;
	DialogNoClickListener(FileSaveDialog sfd)
	{
		SFD = sfd;
	}
	
	public void onClick(DialogInterface dialog, int which) 
	{
		SFD.dismissDialog(FileSaveDialog.DIALOG_FILE_EXISTS);
	}
	
}