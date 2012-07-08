package org.rin;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.app.Activity;
import android.app.ListActivity;

class ExtFilter implements FilenameFilter {
	
	String [] Extensions;
	
	public String[] getExtensions() {
		return Extensions;
	}

	public void setExtensions(String[] extensions) {
		Extensions = extensions;
	}

	public ExtFilter(CharSequence[] charSequences){
		Extensions = (String[]) charSequences;
	}
	
    public boolean accept(File dir, String name) {
   
    	File tmp;
    	try {
    		
    		if(!dir.getCanonicalPath().endsWith("/")){
    			tmp = new File(dir.getCanonicalPath() + "/" + name);
    		}
    		else {
    			tmp = new File(name);
    		}
    		if(tmp.isDirectory())
    		{
    			return true;
    		}
    		else if(tmp.isFile()){
    			for(String ext: Extensions){
    				if(tmp.getName().toLowerCase().endsWith(ext.toLowerCase()) || ext.equals("*")){
    					return true;
    				}
    			}
    			return false;
    		}
		} catch (IOException e) {

			Log.e("FileDialog", "exception in acceptFile trying to get canonical Path for tmp", e);
			return false;
		}
		return false;
		
    }
}
class ClickListener implements OnItemClickListener {
	

	private FileDialog FD;
	private File Parent;
	
	private FileAdapter List;
	

	public ClickListener(FileAdapter list, FileDialog dialog, ExtFilter filter, File parent){
		List = list;
		FD = dialog;
		Parent = parent;
	}

	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		
		if(List.getItem(position).getName().equals(".."))
		{
			if(Parent.getParentFile() != null){
				FD.populateList(Parent.getParentFile());
			}
			
		}
		else
		{	
			if(List.getItem(position).isDirectory()){
				FD.populateList(List.getItem(position));
			}
			else if(List.getItem(position).isFile())
			{	
				FD.onFileClick(position, List);
			}
		}
	}
	
}
class FileAdapter extends ArrayAdapter<File> {

	//File [] items;
	Context Context;
	public FileAdapter(Context context, int textViewResourceId) {
		super(context, textViewResourceId);
		Context = context;
	}


	@SuppressWarnings("static-access")
	@Override
	public View getView(int position, View convertView, ViewGroup parent){
		
		View v = convertView;
		
		if(v == null){
			LayoutInflater vi = (LayoutInflater)Context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			v = vi.inflate(R.layout.fd_row,null); 
		}
		
		
		TextView tv = (TextView) v.findViewById(R.id.row_text);

		if(getItem(position).getName().equals(".."))
		{
				tv.setTextColor(Color.BLUE);
				tv.setText("../ \t\t" + Context.getString(R.string.up));
		}
		else
		{
			
			if(this.getItem(position).isDirectory()){
				tv.setText(getItem(position).getName()+ "/");
				tv.setTextColor(Color.CYAN);
			}
			else
			{
				tv.setTextColor(Color.WHITE);
				tv.setText(getItem(position).getName());
			}
		}

		
		return v;
	}
	
}

public abstract class FileDialog extends ListActivity{
	
	private ListView lv;
	private String startLocation;
	private FileAdapter fileAdapter;

	protected ExtFilter Filter;
	protected CharSequence extFilter[];
	protected String currentDir;
	
	private String root;

	@Override
	public void onCreate(Bundle inState){
		super.onCreate(inState);
	}
		
		
	public void init(Bundle inState)
	{
		if(inState != null)
		{
			startLocation = inState.getString("START_LOCATION");
			root = inState.getString("ROOT");
			currentDir = inState.getString("CUR_DIR");
			extFilter = inState.getCharSequenceArray("EXT_FILTER");
		}
		else
		{
			Bundle bundle = this.getIntent().getExtras();
			startLocation = bundle.getString("startLocation");
			root = bundle.getString("root");
			extFilter = bundle.getCharSequenceArray("extFilter");
			
		}
		if(startLocation == null)
		{
			startLocation = "/";
		}
		if(root == null)
		{
			root = "/";
		}
		if(extFilter == null)
		{
			extFilter = new CharSequence[1];
			extFilter[0] = "";
		}
		if(currentDir == null)
			currentDir = startLocation;

		Filter = new ExtFilter(extFilter);//
		
		fileAdapter = new FileAdapter(this, R.layout.fd_row);

		lv = this.getListView();
		setListAdapter(fileAdapter);
		populateList(new File(currentDir));
	}

	public void populateList(File dir)
	{
		if(!dir.getAbsolutePath().startsWith(root))
		{
			dir = new File(root);
		}
		
		currentDir = dir.getAbsolutePath();
		fileAdapter.clear();
		
		if(!currentDir.equals(root))
		{
			File tmp = new File("../");
			fileAdapter.add(tmp);
		}
		
		File[] flist = null;
		flist = dir.listFiles(Filter);
		
		
		if(flist != null){
			for(int i =0; i< flist.length; i++)
			{
				fileAdapter.add(flist[i]);
			}
			
		}
		lv.setOnItemClickListener(new ClickListener(fileAdapter, this, Filter,dir));

	}
	@Override
	public void onBackPressed()
	{

	}
	@Override 
	public boolean onKeyUp(int keyCode, KeyEvent event) {
	    if (keyCode == KeyEvent.KEYCODE_MENU) {
	        //do nothing
	    }
	    else if(keyCode == KeyEvent.KEYCODE_BACK)
	    {
			Intent tmp = new Intent();
			setResult(Activity.RESULT_CANCELED, tmp);
			finish();
	    }
	    return true;
	}
	
	abstract public void onFileClick(int pos, FileAdapter list);
	
	@Override
	public void onDestroy()
	{
		super.onDestroy();
	}
    @Override
    protected void onSaveInstanceState(Bundle outState)
    {	
    	outState.putString("CUR_DIR", currentDir);
    	outState.putCharSequenceArray("EXT_FILTER",  extFilter);
    	outState.putString("START_LOCATION", startLocation);
    	outState.putString("ROOT",root);
    }
	
}
