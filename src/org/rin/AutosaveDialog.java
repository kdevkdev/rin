package org.rin;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;;


public class AutosaveDialog 
{
	public interface OnAutosaveDialogListener {
		abstract void onCancel();
		abstract void onOk(int id);
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnAutosaveDialogListener listener;
	private AlertDialog dialog;
	private EditText field;

	
	public AutosaveDialog(final Context acontext, Activity anactivity, OnAutosaveDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.autosavedialog, null);
		listener = alistener;
		field = (EditText) view.findViewById(R.id.autosave_menu_time);
		field.setText(Integer.toString(getAutosave()));
		

		
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
				setAutosave(Integer.parseInt(field.getText().toString()));
				listener.onOk(which);
			}
		})
		.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
			    listener.onCancel();
			}
		})
		.setOnCancelListener(new OnCancelListener() {
		    public void onCancel(DialogInterface paramDialogInterface) {
		    	listener.onCancel();
			}
		})
		.create();

		
		// kill all padding from the dialog window
		dialog.setView(view, 0, 0, 0, 0);
	}
	
    public void show(Activity act) 
	{
        dialog.setOwnerActivity(act);
		dialog.show();
	}
	private native int getAutosave();
	private native void setAutosave(int intervall);
}
