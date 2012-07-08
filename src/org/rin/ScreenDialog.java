package org.rin;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.RadioGroup;

public class ScreenDialog 
{
	public interface OnScreenDialogListener {
		abstract void onCancel();
		abstract void onOk(int id);
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnScreenDialogListener listener;
	private AlertDialog dialog;
	private RadioGroup radioGroup;
	
	private final static int SCALE_METHOD_BRESHAM =0;
	private final static int SCALE_METHOD_BILINEAR_GRID = 1;
	private final static int SCALE_METHOD_BILINEAR = 2;
	
	public ScreenDialog(final Context acontext, Activity anactivity, OnScreenDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.screendialog, null);
		listener = alistener;
		
		radioGroup = (RadioGroup) view.findViewById(R.id.scale_radio_group);
		
		
		int id =-1;
		switch(getScaleMethod())
		{
			case SCALE_METHOD_BRESHAM:
				id = R.id.scale_nearest_neighbour;
			break;
			case SCALE_METHOD_BILINEAR_GRID:
				id = R.id.scale_bilinear_grid;
			break;
			case SCALE_METHOD_BILINEAR:
				id = R.id.scale_bilinear;
			break;
		}
		radioGroup.check(id);
		
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
				int id = radioGroup.getCheckedRadioButtonId();
				switch(id)
				{
					case R.id.scale_nearest_neighbour:
						setScaleMethod(SCALE_METHOD_BRESHAM);
					break;
						
					case R.id.scale_bilinear_grid:
						setScaleMethod(SCALE_METHOD_BILINEAR_GRID);
					break;
						
					case R.id.scale_bilinear:
						setScaleMethod(SCALE_METHOD_BILINEAR);
					break;
				}
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
	private native int getScaleMethod();
	private native void setScaleMethod(int id);
}
