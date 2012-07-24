package org.rin;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;


public class TextDialog 
{
	public interface OnTextDialogListener {
		abstract void onOk(int id);
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnTextDialogListener listener;
	private AlertDialog dialog;
	private TextView text;

	
	public TextDialog(final Context acontext, Activity anactivity, OnTextDialogListener alistener, int tid)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.textdialog, null);
		listener = alistener;
		text = (TextView) view.findViewById(R.id.text_text);
		text.setText(Html.fromHtml(activity.getString(tid)));

		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() 
		{
			public void onClick(DialogInterface dialog, int which) 
			{
				listener.onOk(which);
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
}
