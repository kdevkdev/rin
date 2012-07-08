#include "inc/main.h"
#include <time.h>

int pad_state;
int cur_time;
int now_sensor_x,now_sensor_y;

char render_msg[128];
int render_msg_mode;
long render_msg_time;
float timespan = 167;
int lasttime = 0;
unsigned int nextbank =1;


#define GBWIDTH  160
#define GBHEIGHT 144

void render_screen(void *buf)
{
	int now;
	
#ifdef PROFILE
	prof_enter(PF_RENDER_SCREEN);
#endif
	
	if(render_msg_mode!=0 && render_msg_mode!=3){
		pgFillBox(0,263,479,271,0);
		render_msg_mode--;
	}

	//pgBitBltN1(160,64,(unsigned long *)buf);
	//pgBitBlitBilinearT(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	//pgBitBlitBresham(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	//pgBitBlitBilinGrid(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	//pgPrintf(1,20,RGB(255,80,80),"time: %d",getTime());
	//pgBitBlitBilinearS(buf+GGUARD_LINE*2,(unsigned short*)pgGetVramAddr(0, 0), dcomb_wx, dcomb_wy);
	
	switch(setting2.scale_method)
	{
	    case SCALE_METHOD_BRESHAM:
	      pgBitBlitBresham(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	    break;
	    
	    case SCALE_METHOD_BILINEAR_GRID:
	      pgBitBlitBilinGrid(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	    break; 
	    
	    case SCALE_METHOD_BILINEAR:
	      pgBitBlitBilinearT(GBWIDTH, GBHEIGHT, dscreen_width, dscreen_height, (unsigned short *)buf);
	    break; 
	}

	now = getTime();
	timespan = 0.8f*timespan + 0.2f*(now - lasttime);
	lasttime = now;

	if(setting2.show_fps )
	{
	  pgPrintfI(dfps_posx, dfps_posy,"%.1f",10000.0f/timespan);
	}

	
	if(render_msg_mode>=3){
		pgPrint_drawbg(0,50,0xffff,0,render_msg);

		if (getTime() > render_msg_time + 10000)
			render_msg_mode=2;
	}
	
#ifdef PROFILE
	prof_out();
#endif
}


void renderer_update_sound()
{
	//prof_enter(PF_RENDERER_UPDATE_SOUND); (not thread safe)

	unsigned int playbank=(cur_play/sound_buf_len) % BANKS;
	while (nextbank!=(playbank + (BANKS-1))%BANKS) {
	
		snd_render(&sound_buf[nextbank*sound_buf_len],sound_buf_len/2);
		nextbank=((nextbank+1)%BANKS);
	}

	//prof_out();
}
JNIEXPORT void JNICALL Java_org_rin_RinService_setPad(JNIEnv *env, jobject obj, jint new_state)
{
	pad_state = new_state;
}

void renderer_refresh()
{
  /*if(wavout_enable)
  {	
      renderer_update_sound();
  }*/
}

void renderer_init()
{
	cur_time=0;
	render_msg[0]=0;
	render_msg_mode=0;
	lasttime = getTime();
	timespan = 0;
	
	renderer_reset();
}

void renderer_reset()
{
	pad_state=0;
}

uint8_t renderer_get_time(int type)
{
	unsigned long now=time(NULL)-cur_time;

	switch(type){
	case 8: // �b
		return (uint8_t)(now%60);
	case 9: // ��
		return (uint8_t)((now/60)%60);
	case 10: // ��
		return (uint8_t)((now/(60*60))%24);
	case 11: // ��(L)
		return (uint8_t)((now/(24*60*60))&0xff);
	case 12: // ��(H)
		return (uint8_t)((now/(256*24*60*60))&1);
	}
	return 0;
}

void renderer_set_time(int type,byte dat)
{
	unsigned long now=time(NULL);
	unsigned long adj=now-cur_time;

	switch(type){
	case 8: // �b
		adj=(adj/60)*60+(dat%60);
		break;
	case 9: // ��
		adj=(adj/(60*60))*60*60+(dat%60)*60+(adj%60);
		break;
	case 10: // ��
		adj=(adj/(24*60*60))*24*60*60+(dat%24)*60*60+(adj%(60*60));
		break;
	case 11: // ��(L)
		adj=(adj/(256*24*60*60))*256*24*60*60+(dat*24*60*60)+(adj%(24*60*60));
		break;
	case 12: // ��(H)
		adj=(dat&1)*256*24*60*60+(adj%(256*24*60*60));
		break;
	}
	cur_time=now-adj;
}
void renderer_set_bibrate(bool bibrate)
{
}
word renderer_get_sensor(bool x_y)
{
	return (x_y?(now_sensor_x&0x0fff):(now_sensor_y&0x0fff));
}
int renderer_get_timer_state()
{
	return cur_time;
}

void renderer_set_timer_state(int timer)
{
	cur_time=timer;
}

void renderer_set_msg(const char msg[])
{
	render_msg_mode = 6;
	render_msg_time = getTime();
	strcpy(render_msg, msg);
}

