#include "inc/main.h"

unsigned long cur_play=0;
int wavout_enable=1;

void wavoutClear()
{

	//apu_init();
	snd_clear();
	memset(sound_buf, 0, SOUND_BUF_LEN*BANKS*sizeof(short));
}

JNIEXPORT jint JNICALL Java_org_rin_RinAudioRunnable_requestChunk(JNIEnv *enva, jobject this, jshortArray audioArray, jint size)
{
	int endpos = cur_play + size;
	

	if(wavout_enable)
	{
		renderer_update_sound();
				
		if(endpos >= BANKS*sound_buf_len)
		{
			(*enva)->SetShortArrayRegion(enva, audioArray, 0,(BANKS*sound_buf_len)- cur_play, (short *) &sound_buf[cur_play]);
			(*enva)->SetShortArrayRegion(enva, audioArray, (BANKS*sound_buf_len)- cur_play,endpos -  (BANKS*sound_buf_len), (short *) &sound_buf[0]);
		}
		else
		{
			(*enva)->SetShortArrayRegion(enva, audioArray, 0,size, (short *) &sound_buf[cur_play]);
		}
	}

	
	cur_play = (cur_play + size)% (BANKS*sound_buf_len);
	return 1;
}

