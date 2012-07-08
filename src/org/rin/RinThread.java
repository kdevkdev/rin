package org.rin;

class RinThread extends Thread
{
		private boolean mRunning;
		private String rinPath;
		
		//params for the native function
    	
    	public RinThread()
    	{

    		mRunning = false;
    		setPriority(MAX_PRIORITY);
    		setName("rin game thread");
    	}
    	
    	@Override
    	public void run()
    	{
    		if(rinPath.length() >0)
	    	{
	    		mRunning = true;
	    		startNative(rinPath);
	    	}
    	}
		
		public void stopGame()
		{
			stopNative();
			mRunning = false;
		}

		private native int startNative(String rpath);
	    private native int stopNative();

		public boolean isRunning() {
			return mRunning;
		}

		public void setRinPath(String rpath) {
			rinPath = rpath;
			
		}
    }