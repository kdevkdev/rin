package org.rin;



import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

public class SpeedDialog 
{
	public interface OnSpeedDialogListener {
		abstract void onCancel();
		abstract void onOk(int id);
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnSpeedDialogListener listener;
	private AlertDialog dialog;
	private Button DefaultButton;
	private SeekBar seekBar; 
	private TextView speedValue;
	
	private final static int SCALE_METHOD_BRESHAM =0;
	private final static int SCALE_METHOD_BILINEAR_GRID = 1;
	private final static int SCALE_METHOD_BILINEAR = 2;
	
	public SpeedDialog(final Context acontext, Activity anactivity, OnSpeedDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.speeddialog, null);
		listener = alistener;
		DefaultButton = (Button) view.findViewById(R.id.speed_default_button);
		seekBar = (SeekBar) view.findViewById(R.id.speed_seek_bar);
		speedValue = (TextView) view.findViewById(R.id.speed_value);
		seekBar.setProgress(getSpeed());
		speedValue.setText(Integer.toString(seekBar.getProgress()));
		
		DefaultButton.setOnTouchListener(new OnTouchListener(){

			public boolean onTouch(View arg0, MotionEvent arg1) 
			{
				seekBar.setProgress(30);
				return false;
			}});
		
		seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			public void onProgressChanged(SeekBar seekbar, int progress, boolean fromUser) {
				speedValue.setText(Integer.toString(progress));
			}

			public void onStartTrackingTouch(SeekBar arg0) {
				
			}

			public void onStopTrackingTouch(SeekBar arg0) {

				
			}});

		
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
				setSpeed(seekBar.getProgress());
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
	private native int getSpeed();
	private native void setSpeed(int speed);
}
