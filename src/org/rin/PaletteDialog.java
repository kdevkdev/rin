package org.rin;

import org.rin.PaletteColorPick.OnPaletteColorPickListener;

import yuku.ambilwarna.R;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

public class PaletteDialog
{
	public interface OnPaletteDialogListener {
		abstract void onCancel();
		abstract void onOk(int id);
	}
	
	final AlertDialog dialog;
	public final Bitmap.Config BITMAP_CONFIG = Bitmap.Config.RGB_565;
	Context context;
	View view;
	View explaintext;
	Activity activity;
	PaletteColorPick colorDialog;
	OnPaletteDialogListener listener;
	Spinner spinner;
	WindowManager.LayoutParams params;
	
	//color values, they must correspond to the values in lcd.c and gb.h
	final static int PAL_CUSTOM_LOCAL = 13;
	final static int PAL_CUSTOM_GLOBAL = 14;
	final static int PAL_SGB = 15;
	
	public PaletteDialog(final Context acontext, Activity anactivity, OnPaletteDialogListener alistener)
	{
		context = acontext;
		activity = anactivity;
		view = LayoutInflater.from(context).inflate(R.layout.palettedialog, null);
		explaintext = view.findViewById(R.id.explaintext_color_dialog);
		listener = alistener;
		
	
		
	    spinner = (Spinner) view.findViewById(R.id.pals_predefined_spinner);
	    
	    PaletteAdapter adapter = new PaletteAdapter(context, R.array.palettes_array, android.R.layout.simple_spinner_item);
	    
	    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(adapter);

	    Resources res = context.getResources();
	    String[] paletteStrings = res.getStringArray(R.array.palettes_array);
	    PaletteItem paletteItem;
	    
	    for(int i= 0; i< paletteStrings.length; i++)
	    {
	    	paletteItem = new PaletteItem(paletteStrings[i], i);
	    	adapter.add(paletteItem);
	    }
	    int palette = getSelectedPalette();
	    spinner.setSelection(palette);
	    setSelectColors(palette);
	    spinner.setOnItemSelectedListener(new OnItemSelectedListener() {

			public void onItemSelected(AdapterView<?> parent, View view, int position,
					long id) {
				
					setSelectColors(position);
				}

			public void onNothingSelected(AdapterView<?> parent) {

				
			}
	    	 });
	   
	    
		dialog = new AlertDialog.Builder(context)
		.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				setPalette(spinner.getSelectedItemPosition());
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
		
		   params = new WindowManager.LayoutParams();
		    params.copyFrom(dialog.getWindow().getAttributes());
		    params.width = WindowManager.LayoutParams.WRAP_CONTENT;
		    params.height = WindowManager.LayoutParams.WRAP_CONTENT;

	}

	private void setSelectColors(int pal)
	{
		int background;
		
		if(pal == PAL_CUSTOM_LOCAL || pal == PAL_CUSTOM_GLOBAL)
		{
			background = R.drawable.color_select_background;
			explaintext.setVisibility(View.VISIBLE);
		}
		else
		{
			background = 0;
			explaintext.setVisibility(View.INVISIBLE);
		}

		
	    for(int i =0; i< 3; i++)
	    {
	    	for(int j =0; j< 4; j++)
	    	{
	    		String name = "pal_select_img_"+(i+1) + (j+1);
	    		int id = context.getResources().getIdentifier(name, "id", "org.rin");
	    	
	    		ImageView iv = (ImageView) view.findViewById(id);
	    		iv.setBackgroundResource(background);

	    		Bitmap bitmap = Bitmap.createBitmap(20, 20, BITMAP_CONFIG);
	    		Canvas c = new Canvas(bitmap);
	    		Paint p =  new Paint();

	    		int col = getPaletteColor(pal,i,j);
	    		int ncol = (255 << 24) | (gbvRed(col) << 16) | (gbvGreen(col) << 8) | gbvBlue(col);
	    		p.setARGB(255, gbvRed(col), gbvGreen(col), gbvBlue(col));
	    		c.drawRect(0, 0, 50, 50,p);
	    		
	    		iv.setImageBitmap(bitmap);
	    		
	    		
	    		if(pal == PAL_CUSTOM_LOCAL)
	    		{
		    		colorDialog = new PaletteColorPick(context, activity,bitmap, iv, ncol,i,j, new OnPaletteColorPickListener(){

						public void onCancel()
						{
							
						}

						public void onOk(int color, int row, int column, Bitmap bitmap, ImageView iv)
						{
							int r = vRed(color);
							int g = vGreen(color);
							int b = vBlue(color);
							setCustomLocalPaletteColor(row, column, r, g, b);
						
							Paint p = new Paint();
							Canvas c = new Canvas(bitmap);
							p.setARGB(255,r,g,b);
							c.drawRect(0,0,50,50, p);
							
				    		iv.setImageBitmap(bitmap);
							
						
						}});
		    		iv.setOnClickListener(colorDialog);
	    		}
	    		else if(pal == PAL_CUSTOM_GLOBAL)
	    		{
		    		colorDialog = new PaletteColorPick(context, activity,bitmap, iv, ncol,i,j, new OnPaletteColorPickListener(){

		    				public void onCancel()
							{
								
							}
		    			
							public void onOk(int color, int row, int column, Bitmap bitmap, ImageView iv)
							{
							int r = vRed(color);
							int g = vGreen(color);
							int b = vBlue(color);
							setCustomGlobalPaletteColor(row, column, r, g, b);
						
							Paint p = new Paint();
							Canvas c = new Canvas(bitmap);
							p.setARGB(255,r,g,b);
							c.drawRect(0,0,50,50, p);
							
				    		iv.setImageBitmap(bitmap);
							
						
						}});
	    		iv.setOnClickListener(colorDialog);
	    		}
	    	}
	    }
	}
	//these are for bgr 555
    int gbvRed(int c)
    {
    	return ((c & 0x1F) << 3);
    }
    int gbvGreen(int c)
    {
    	return ((c & 0x03E0) >> 2);
    }
    int gbvBlue(int c)
    {
    	return ((c & 0x7C00) >> 7);
    }
    //those are for argb 8888
    int vRed(int c)
    {
    	return ((c & 0xFF0000) >> 16);
    }
    int vGreen(int c)
    {
    	return ((c & 0xFF00) >> 8);
    }
    int vBlue(int c)
    {
    	return ((c & 0xFF));
    }
    public void show(Activity act) 
	{
        dialog.setOwnerActivity(act);
		dialog.show();
		
		//dialog.getWindow().setAttributes(params);
	}
	
	class PaletteItem
	{
		String name;
		int id;
		public PaletteItem(String aname, int aid)
		{
			id = aid;
			name = aname;
		}
	}
	class PaletteAdapter extends ArrayAdapter<PaletteItem> 
	{
		public final Bitmap.Config BITMAP_CONFIG = Bitmap.Config.RGB_565;
		Bitmap palbitmaps[];
		public PaletteAdapter(Context acontext,int itemsId, int resourceId) {
			super(acontext, resourceId);
			palbitmaps = new Bitmap[15];
			
			
			for(int k = 0; k < 15; k++)
			{
				palbitmaps[k] = Bitmap.createBitmap(160, 120, BITMAP_CONFIG);
				Canvas c = new Canvas(palbitmaps[k]);
				Paint p =  new Paint();
				
				for(int i =0; i< 3; i++)
				{
					for(int j =0; j<4;j++)
					{
						
			    		int col = getPaletteColor(k,i,j);
			    		p.setARGB(255,gbvRed(col), gbvGreen(col), gbvBlue(col));
						c.drawRect(j*40, i*40, (j+1)*40, (i+1)*40,p);
					}
				}
			}
		}
		
		@Override
		public View getDropDownView(int position, View convertView, ViewGroup parent)
		{
			View v = convertView;
			if(v == null){
				LayoutInflater vi = (LayoutInflater)context.getSystemService(android.content.Context.LAYOUT_INFLATER_SERVICE);
				v = vi.inflate(R.layout.pal_spinner_row,null); 
			}
			
			TextView tv = (TextView) v.findViewById(R.id.pal_spinner_row_text);
			tv.setText(getItem(position).name);
			
			ImageView iv = (ImageView) v.findViewById(R.id.pal_spinner_row_image);
			iv.setImageBitmap(palbitmaps[position]);
			
			return v;
		}
		@Override
		public View getView(int position, View convertView, ViewGroup parent){
			
			View v = convertView;
			
			if(v == null){
				LayoutInflater vi = (LayoutInflater)context.getSystemService(android.content.Context.LAYOUT_INFLATER_SERVICE);
				v = vi.inflate(R.layout.pal_spinner_row,null); 
			}
			
			
			TextView tv = (TextView) v.findViewById(R.id.pal_spinner_row_text);
			tv.setText(getItem(position).name);
			ImageView iv = (ImageView) v.findViewById(R.id.pal_spinner_row_image);
			iv.setImageBitmap(palbitmaps[position]);

			
			return v;
		}
		
	}
	
	private native int setCustomLocalPaletteColor(int row, int column, int red, int green, int blue);
	private native int setCustomGlobalPaletteColor(int row, int column, int red, int green, int blue);
	
	
	private native int getPaletteColor(int pal, int row, int column);
	private native int getSelectedPalette();
    public native void setPalette(int id);
}

