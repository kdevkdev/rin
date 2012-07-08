package org.rin;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import yuku.ambilwarna.AmbilWarnaDialog;
import yuku.ambilwarna.AmbilWarnaDialog.OnAmbilWarnaListener;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;

public class PaletteColorPick implements OnClickListener {
	public interface OnPaletteColorPickListener {
		abstract void onCancel();
		abstract void onOk( int color, int row, int column, Bitmap bitmap, ImageView iv);
	}
	
	AmbilWarnaDialog warnaDialog;
	int row,  column;
	OnPaletteColorPickListener listener;
	Activity activity; 
	Bitmap bitmap;
	ImageView imageView;
	public PaletteColorPick(Context context,Activity anactivity,Bitmap abitmap, ImageView iv, int color, int i, int j,
			OnPaletteColorPickListener alistener) 
	{
		activity = anactivity;
		listener = alistener;
		imageView = iv;
		row = i;
		column = j;
		bitmap = abitmap;
		warnaDialog = new AmbilWarnaDialog(context, color, new OnAmbilWarnaListener(){

			public void onCancel(AmbilWarnaDialog dialog) {
				listener.onCancel();
			}

			public void onOk(AmbilWarnaDialog dialog, int color) {
				listener.onOk(color, row, column, bitmap, imageView);
				
			}});
	}

	public void onClick(View v) {
		warnaDialog.show(activity);
	}

}
