package org.rin;
import java.util.concurrent.atomic.AtomicInteger;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;


public class RinAudioRunnable extends AudioTrack implements Runnable
{

	private static final int samplerate = 44100;

	private  int bankSize;

	static private int BigBufferSizeByte= getMinBufferSize(samplerate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
	
	//must be multiple of bankSize
	int BigBufferSize;
	short [] audioBuffer;
	
	final int STATE_PLAYING = 2;
	final int STATE_STOPPED = 3;
	final int STATE_TERMINATED = 4;
	
	AtomicInteger state;
	AtomicInteger stateReq;
	boolean waitState;
	
	private volatile boolean running = true;
	
	private Object lock;
	
	public RinAudioRunnable(int abufferSize) throws IllegalArgumentException 
	{

		super(AudioManager.STREAM_MUSIC, samplerate,  AudioFormat.CHANNEL_OUT_STEREO,  AudioFormat.ENCODING_PCM_16BIT,
				BigBufferSizeByte, AudioTrack.MODE_STREAM);
		
		@SuppressWarnings("unused")
		int minBufferSize = getMinBufferSize(samplerate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
		

		state = new AtomicInteger(STATE_STOPPED);
		stateReq = new AtomicInteger(0);
		waitState =  true;
		
		lock = new Object();
		
		BigBufferSize = BigBufferSizeByte/2;
		
		bankSize = abufferSize;
		audioBuffer = new short[bankSize];
		
		if(getState() != AudioTrack.STATE_INITIALIZED)
		{
			throw new IllegalArgumentException();
		}
		//log.d("org.rin", "audio roennabLE construcor");
	}
	public int startAudio()
	{
		//log.d("org.rin", "RinAudioRunnable: startAudio()");
		synchronized(lock)
		{
			if(state.get() == STATE_STOPPED)
			{
				stateReq.set(STATE_PLAYING);
				
				lock.notify();
	
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}

	public  int stopAudio()
	{
		//log.d("org.rin", "RinAudioRunnable: stopAudio()");
		if(state.get()==STATE_PLAYING)
		{
			stateReq.set(STATE_STOPPED);			

			return 1;
		}
		else
		{
			return 0;
		}
	}
	public void terminateAudio()
	{
		//log.d("org.rin", "RinAudioRunnable: terminateAudio()");
		synchronized(lock)
		{
			if(state.get() == STATE_STOPPED)
			{
				stateReq.set(STATE_TERMINATED);
				lock.notify();
			}
		}
	}
	public void run() 
	{
		//log.d("org.rin", "RinAudioRunnable: run() - audiothread started");
		while(running)
		{
			synchronized(lock)
			{
				if(waitState)
				{
					try
					{
						//log.d("org.rin","RinAudioRunnable: run() - starting to wait in try block");	
						lock.wait();
						//log.d("org.rin","RinAudioRunnable: run() - wait finished");	

					}
					catch(Exception e)
					{
						//log.e("org.rin","RinAudioRunnable: run() - exception while waiting with waitState:"+ e.getMessage());
					}
					waitState = false;
				}
			}
				
			
			int tempState = state.get();
			int tempReqState = stateReq.get();
			if(tempState == STATE_TERMINATED)
			{
				//do nothing
			}
			else if(tempState == STATE_PLAYING)
			{
				if(tempReqState == STATE_STOPPED)
				{
					pause();
					//log.d("org.rin", "RinAudioRunnable: run() - audio stopped");
					
					//wait on restart or temination
					waitState = true;
					
					
					//set states
					state.set(STATE_STOPPED);
					stateReq.set(0);
				}
				else if(tempReqState == 0)
				{
					//continue writing to sink
		        	requestChunk(audioBuffer, bankSize);
		        	write(audioBuffer, 0, bankSize);
				}
				else
				{
					//illegal value, reset to 0
					stateReq.set(0);
				}
			}
			else if(tempState == STATE_STOPPED)
			{
				if(tempReqState == STATE_PLAYING)
				{
					play();
		        	requestChunk(audioBuffer, bankSize);
		        	write(audioBuffer, 0, bankSize);
		       
		    		//log.d("org.rin", "first written audio");
		        	
		        	//set states
		        	state.set(STATE_PLAYING);
		        	stateReq.set(0);
				}
				else if(tempReqState == 0)
				{
					//remain paused
					waitState = true;
				}
				else if(tempReqState == STATE_TERMINATED)
				{
					//cancell thread, stop audio playback
					flush();
					stop();
					release();
					pgaTerm();
					running = false;
					
					//set states
					state.set(STATE_TERMINATED);
					stateReq.set(0);
				}
				else
				{
					//illegal value, reset to 0
					stateReq.set(0);
				}
			}
	
		}
		//log.d("org.rin", "audio thread exited");
	}
	public short[] getAudioBuffer()
	{
		return audioBuffer;
	}
	
	//Attention: the requestchunk size must fit into of banks * sound_buff_len
	public native int requestChunk(short[] audioArray,int size);
	private native void pgaTerm();
	
}
