package org.rin;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import android.util.Log;

public class cFile 
{
	//provides easy gzip and zip support based on file extension with C- like interface
	static final int MODE_NEW = 1;
	static final int MODE_READ = 2;
	static final int MODE_OVERWRITE = 4;
	static final int MODE_WRITE = 8;

	
	FileInputStream instream;
	FileOutputStream ostream;
	BufferedInputStream binstream;
	BufferedOutputStream boutstream;
	ZipEntry zentry;
	File file;
	String [] zendings;
	String name;
	String path;
	int mode;
	int ok;
	int sizeCompressed;
	int sizeUncompressed;
	
	public int getSizeCompressed() 
	{
		return sizeCompressed;
	}
	public int getSizeUncompressed() 
	{
		return sizeUncompressed;
	}


	ZipOutputStream zout;
	public cFile(String apath, int amode, String [] zends) 
	{	
		path = apath;
		mode = amode;
		name = path.substring(path.lastIndexOf("/")+1);
		zout = null;
		zentry = null;
		ok = 1;
		sizeCompressed = 0;
		sizeUncompressed =0;
		
		if(name.endsWith(".zip"))
		{
			name = name.substring(0, name.lastIndexOf(".zip"));
		}
		
		zendings = new String[zends.length];
		System.arraycopy(zends, 0, zendings, 0, zends.length);
		
		file = new File(path);
		instream = null;
		ostream = null;
		
		if((mode & (MODE_NEW )) !=0 )
		{
			try 
			{
				if(file.exists())
				{
					if((mode & MODE_OVERWRITE) != 0)
					{
							file.delete();
							file.createNewFile();
					}
				}
				else
				{
					file.createNewFile();
				}
			}
			catch (IOException e) 
			{
				Log.e("org.rin", "cFile: error creating file", e);
				ok = -1;
			}
		}

		try 
		{
			if((mode & (MODE_READ)) !=0 )
			{
				instream = new FileInputStream(file);
				if(path.endsWith(".zip"))
				{
					ZipInputStream zin = new ZipInputStream(instream);

					zentry = zin.getNextEntry();
					boolean cont = true;
					while(zentry != null && cont)
					{
						for(String item:zendings)
						{
							if(zentry.getName().endsWith(item))
							{
								cont = false;
							}	
						}
						if(cont)
						{
							zentry = zin.getNextEntry();
						}
					}
					if(cont)
					{
						//Log.d("org.rin","cfile: no files in zip with one of the requested endings found");
						ok = -2;
					}
					binstream = new BufferedInputStream(zin);
				}
				else if(path.endsWith(".gz"))
				{
					GZIPInputStream gin = new GZIPInputStream(instream);
					binstream = new BufferedInputStream(gin);
				}
				else
				{
					binstream = new BufferedInputStream(instream);
				}
				calculateSize();
			}
			if((mode & (MODE_WRITE)) != 0)
			{
				ostream = new FileOutputStream(file);
				
				if(path.endsWith(".zip"))
				{
					zout = new ZipOutputStream(ostream);
					zout.putNextEntry(new ZipEntry(name));
					
					boutstream  = new BufferedOutputStream(zout);
				}
				else if(path.endsWith(".gz"))
				{
					GZIPOutputStream gout = new GZIPOutputStream(ostream);
					boutstream  = new BufferedOutputStream(gout);
				}
				else
				{
					boutstream  = new BufferedOutputStream(ostream);
				}
			}
		} 
		catch (FileNotFoundException e) 
		{
			//Log.d("org.rin","cfile: error opening file (not found)",e);
			ok = -3;
		} 
		catch (IOException e) 
		{
			Log.e("org.rin","cfile: error while opening some compressed stream",e);
			ok = -4;
		}
	}
	public int getOk()
	{
		return ok;
	}
	private void calculateSize()
	{
		//only makes sense for reading files
		if(path.endsWith(".zip"))
		{
			sizeCompressed = (int) zentry.getCompressedSize();
			sizeUncompressed = (int) zentry.getSize();
		}
		else if(path.endsWith(".gz"))
		{
			sizeCompressed = (int) file.length();
			
			
			int read;
			try 
			{
				byte[] buffer = new byte[1024];
				//hackily calculate filesize
				File tfile = new File(path);
				FileInputStream tins = new FileInputStream(tfile);
				GZIPInputStream tgins = new GZIPInputStream(tins);
				BufferedInputStream inr  = new BufferedInputStream(tgins);
				read = inr.read(buffer,0, 1024);

				while(read > 0)
				{
					sizeUncompressed += read;
					read = inr.read(buffer,0,1024);
				}
				inr.close();
			} 
			catch (IOException e) 
			{
				//Log.e("org.rin","cFile: error calculating gzip filesize uncompessed");
			}
			
		}
		else
		{
			sizeCompressed = (int) file.length();
			sizeUncompressed = (int) file.length();
		}
	}
	public int read(byte[] buf,int of, int max)
	{
		try 
		{
			return binstream.read(buf, of, max);
		} 
		catch (IOException e) 
		{
			Log.e("org.rin","cfile: ioexception in read ",e);
			return -1;
		}
	}
	public void  write(byte[] buf, int of, int length)
	{
		try 
		{
			boutstream.write(buf, of, length);
		} 
		catch (IOException e) 
		{
			Log.e("org.rin","cfile: ioexception in write",e);
		}
	}
	public void close( )
	{

		try
		{
			if(zout != null)
			{
				zout.closeEntry();
			}
			
			if(binstream != null)
			{
				binstream.close();
			}
			if(boutstream != null)
			{
				boutstream.close();
			}
		}
		catch(IOException e)
		{
			Log.e("org.rin", "cfile: error closing buffered stream", e);
		}
	}
}