package org.rin;

public class RinAudio 
{
	RinAudioRunnable AudioRunnable;
	Thread AudioThread;
	
	public RinAudio(int abufferSize)
	{
		AudioRunnable = new RinAudioRunnable(abufferSize);
		AudioThread = new Thread(AudioRunnable);
		AudioThread.setName("rin audio thread");
		AudioThread.start();
	}

	public void stopAudio() 
	{
		AudioRunnable.stopAudio();
	}

	public void startAudio() 
	{
		AudioRunnable.startAudio();
	}

	public void terminateAudio() 
	{	
		AudioRunnable.terminateAudio();
	}
}
