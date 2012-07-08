package org.rin;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;


public class HelpDialog 
{
	public interface OnHelpDialogListener {
		abstract void onOk(int id);
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnHelpDialogListener listener;
	private AlertDialog dialog;
	private TextView text;

	
	public HelpDialog(final Context acontext, Activity anactivity, OnHelpDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.helpdialog, null);
		listener = alistener;
		text = (TextView) view.findViewById(R.id.help_text);
		text.setText(Html.fromHtml(activity.getString(R.string.help_menu_text)));

		
		
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
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
