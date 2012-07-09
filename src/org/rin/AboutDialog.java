package org.rin;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.text.method.ScrollingMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;


public class AboutDialog 
{
	public interface OnAboutDialogListener {
		abstract void onOk(int id);
		abstract void onCancel();
	}
	private Context context;
	private Activity activity; 
	private View view;
	private OnAboutDialogListener listener;
	private AlertDialog dialog;
	private TextView text;

	
	public AboutDialog(final Context acontext, Activity anactivity, OnAboutDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.aboutdialog, null);
		listener = alistener;
		text = (TextView) view.findViewById(R.id.about_text);
		
		String dontext = activity.getString(R.string.donate);
		String link = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=HL84X9ZRHNURL&lc=CH&item_name=kdevkdev&item_number=arin&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted";
		text.setText(Html.fromHtml(activity.getString(R.string.about_menu_text)+ "<br><br><a href=\"" + link + "\">" + dontext + "</a>"));
		
		text.setMovementMethod(LinkMovementMethod.getInstance());
		
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
				listener.onOk(which);
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
}
