package org.rin;

import java.io.IOException;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;


public class FileLoadDialog extends FileDialog
{
	
	@Override
	public void onCreate(Bundle inState)
	{
			super.onCreate(inState); 
			setContentView(R.layout.filedialog_load);
			init(inState);

	}

	@Override
	public void onFileClick(int pos, FileAdapter list) 
	{

		boolean found = false;
		for(String ext:Filter.getExtensions()){
			if(list.getItem(pos).getName().toLowerCase().endsWith(ext.toLowerCase()))
			{
				found = true;
			}
		}
		if(found)
		{
			Intent tmp = new Intent();
			try {
				tmp.putExtra("path", list.getItem(pos).getCanonicalPath());
				setResult(Activity.RESULT_OK, tmp);
			} catch (IOException e) {
				setResult(Activity.RESULT_CANCELED, tmp);
				//log.e("FileDialog", "exception in onItemClick trying to get canonical Path for result", e);
			}
			finally
			{
				finish();
			}
		}
		else
		{
			Notification.showNotification(this,getString(R.string.invalid_file_extension));
		}
	    if(!isFinishing()){
	    	Notification.showNotification(this,getString(R.string.invalid_file));
	    }
	}
}
