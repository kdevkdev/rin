#include <android/bitmap.h>

#include "inc/main.h"


#define STATE_PAUSED      1
#define STATE_EMULATING   2
#define STATE_SLEEPING    3
#define STATE_EXITED	  4

int state;
int state_req;

//word *gb_video;//This needs to be of size GLINE_SIZE*GCOLUMN_SIZE;


int rin_rewind = 0;
int bMenu=0;
int running = 1;
int exited = 0;

char AutosavePath[PATH_MAX];
char SramPath[PATH_MAX];
char RinPath[PATH_MAX];
char RomPath[PATH_MAX];
char RomName[MAX_NAME];
char SavePath[PATH_MAX];
char CheatPath[PATH_MAX];
char ScreenPath[PATH_MAX];


queue_t * order_queue;

pthread_mutex_t state_mutex      = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  state_cond_var   = PTHREAD_COND_INITIALIZER;

pthread_mutex_t sleep_mutex      = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  sleep_cond_var   = PTHREAD_COND_INITIALIZER;


struct timeval StartTime;
unsigned int last_autosave =0;



//returns time in 1/10000 seconds since programm start
unsigned int getTime()
{
	/*struct timeval t;
	gettimeofday(&t,NULL);
	return (t.tv_sec-StartTime.tv_sec) * 10000 + t.tv_usec-StartTime.tv_usec/100;*/

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec*10000 + (now.tv_nsec/100000);

}
//programm running time in secons
unsigned int upTime()
{
  	struct timeval t;
	gettimeofday(&t,NULL);
	return (t.tv_sec-StartTime.tv_sec);
}
int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1 && errno == EINTR)
         continue;
    return 1;
}

//******************* profiling stuff *****************************
//use just functions in one thread (for example requestChunk wouldnt work without special treatment)


#ifdef PROFILE
int last_prof_time;
FILE * pFile;
struct prof_struct {
	int func;
	int start_time;
	struct  prof_struct * prev;
}; 

struct prof_struct * prof_stack_top;
char functs[256][32];
int proft[256];
int profc[256];
void prof(char * message)
{
	int tmp = getTime();
	fprintf(pFile,"%s: %d\n",message, tmp - last_prof_time);
	last_prof_time = tmp;
}
void prof_init()
{

	int i =0;
	while(i < 255)
	{
		proft[i] = 0;
		profc[i] = 0;
		functs[i][0] = 0;
		i++;
	}
	strcpy(functs[PF_GB_RUN],"gb_run");
	strcpy(functs[PF_RENDER_SCREEN],"render_screen");
	strcpy(functs[PF_BIG_BLIT_STFULL],"bigBlitStFull");
	strcpy(functs[PF_BIG_BLIT_BILIN],"bigBlitBilinear");
	strcpy(functs[PF_BIG_BLIT_BILIN_GRID],"bigBlitBilinGrid");
	strcpy(functs[PF_BIG_BLIT_BRESHAM],"bigBlitBresham");
	strcpy(functs[PF_RENDERER_UPDATE_SOUND],"renderer_update_sound");
	strcpy(functs[PF_PG_SCREEN_FLIP],"pgScreenFlip");
	strcpy(functs[PF_CPU_EXEC],"cpu_exec");
}
void prof_enter(int func)
{
	struct prof_struct * temp = prof_stack_top;
	prof_stack_top = malloc(sizeof(struct prof_struct));

	if(!prof_stack_top)
		exit(EXIT_FAILURE);

	prof_stack_top->start_time = getTime();
	prof_stack_top->func = func;
	prof_stack_top->prev = temp;
}
void prof_out()
{
	proft[prof_stack_top->func] += (getTime()-prof_stack_top->start_time)/10.0f;
	profc[prof_stack_top->func] ++;

	struct prof_struct * temp = prof_stack_top;
	
	if(prof_stack_top->prev)
	{
		prof_stack_top = prof_stack_top->prev;

	}
	free(temp);
}
void prof_print()
{
	int i = 0;
	
	if(pFile == 0)
	{
		return;
	}
	
	while(i < 255)
	{
		if(profc[i]>0)
		{
			  
			fprintf(pFile,"%s: \t %d ms / \t %d times =  \t %f ms\n",functs[i], proft[i],profc[i], (float)proft[i]/(float)profc[i]);
		}
		i++;	
	}
}
void plog(char * message, int number)
{
	fprintf(pFile,"%s: %d\n",message, number);
}

#endif
//******************************************************************
void mainloop(void)
{


	int framecount = 0;
	int save_period_frames = 10;

	
	unsigned long now_time = getTime();
	unsigned long now_time_bak = now_time;
	unsigned long prev_time = now_time;
	unsigned long next_time = now_time + sync_time;
	unsigned long waitfc=0;
	int line, turbo_bak=0;

	sync_time = 167;
	slow_mod  = 1;

	
	char battery[32];
	int ret;

	
	while(running) 
	{
		pthread_mutex_lock(&state_mutex);
		if(state_req)
		{
			if(state_req == STATE_SLEEPING && state == STATE_PAUSED)
			{
				state = STATE_SLEEPING;
				pthread_cond_broadcast(&state_cond_var);
				pthread_mutex_unlock(&state_mutex);

				pthread_mutex_lock(&sleep_mutex);
				while(state_req != STATE_PAUSED)
					pthread_cond_wait(&sleep_cond_var,&sleep_mutex);
				pthread_mutex_unlock(&sleep_mutex);

				pthread_mutex_lock(&state_mutex);
				state = STATE_PAUSED;
				pthread_cond_broadcast(&state_cond_var);

				now_time = getTime();
				prev_time = now_time;
				next_time = now_time + sync_time;
				skip=0;
			}
			else if(state_req == STATE_PAUSED && state == STATE_EMULATING)
			{
				state = STATE_PAUSED;
				pthread_cond_broadcast(&state_cond_var);

			}
			else if(state_req == STATE_EMULATING && state == STATE_PAUSED)
			{
				state = STATE_EMULATING;
				pthread_cond_broadcast(&state_cond_var);

			}
			else if(state_req == STATE_EXITED)
			{
			  	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native exit request detectect - leaving");
				running = 0;
				state = STATE_EXITED;
				pthread_mutex_unlock(&state_mutex);
				continue;
			}
			state_req = 0;
		}
		pthread_mutex_unlock(&state_mutex);
		process_orders();



		now_time = getTime();
	



		if (bTurbo)
		{
			turbo_bak = 1;
			skip++;
			if (skip > 9){
				skip = 0;
				prev_time = now_time;
			}
		}
		else if (now_time < now_time_bak)
		{
			prev_time = now_time;
			skip=0;
		}
		else if (now_time > next_time){
			skip++;

			if(skip > setting.frameskip){
				skip=0;

				prev_time = now_time;

			}
		}
		else
		{

			waitfc++;
			while(now_time < prev_time+10*(skip+1))
			{
				now_time = getTime();

				sched_yield();
			}
			/*if (!(waitfc&3))
			{
				while(now_time < next_time)
				{
					now_time = getTime();
				}
			}*/
			prev_time = next_time;

			skip=0;
		}

		now_time_bak = now_time;
		next_time = prev_time + sync_time * slow_mod * (skip+1);
		if(rom_get_loaded())
		{
			if(state == STATE_EMULATING)
			{
				for(line=0; line<154; line++)
					gb_run();

				framecount++;
				if(framecount == save_period_frames)
				{
					save_rewind_state();
					framecount = 0;
				}

				if(rin_rewind)
				{
					while(rin_rewind)
					{
						skip=0;
						now_frame =0;
					
						//begin rewinds
						if(read_rewind_state() > 0 ){
						
							for(line=0; line<154; line++) //emulate a frame
								gb_run();
								
						}
						else
						{
							rin_rewind = false;
							continue;
						}
						msleep(24);
						pgScreenFlip();
						process_orders();
					}
					
					wavoutClear();
				
					//continue normal emulation
					if(setting.sound) 
					{
						wavout_enable=1;
					}

					now_time = getTime();
					prev_time = now_time;
					next_time = now_time + sync_time;
					skip=0;
				}
				if(setting.autosave)
				{
				    if(last_autosave + setting.autosave < upTime())
				    {
					//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native autosave %s", AutosavePath);
					last_autosave = upTime();
					state_save(AutosavePath);
				    }
				}
			}
			else
			{
			      usleep(30);
			}
		}



		if (!bTurbo){
			if (turbo_bak)
				turbo_bak = 0;
			else if (skip==0) {
				pgScreenFlip();
			}
		}else{
			if (now_frame==0) {
				pgScreenFlip();
			}
		}
		

	}
}
JNIEXPORT void JNICALL Java_org_rin_RinService_enableWav(void)
{
	wavout_enable = 1;
}
JNIEXPORT void JNICALL Java_org_rin_RinService_disableWav(void)
{
	wavout_enable = 0;
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_pauseNative(void)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native pauseNative()");
	int result = 0;
	pthread_mutex_lock(&state_mutex);
	if(state == STATE_EMULATING)
	{
		state_req = STATE_PAUSED;
		pthread_cond_wait(&state_cond_var,&state_mutex);
		result = 1;
	}
	pthread_mutex_unlock(&state_mutex);
	return result;
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_unpauseNative(void)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native unpauseNative()");
	int result = 0;
	pthread_mutex_lock(&state_mutex);
	if(state == STATE_PAUSED)
	{
		state_req = STATE_EMULATING;
		pthread_cond_wait(&state_cond_var,&state_mutex);
		result = 1;
	}
	pthread_mutex_unlock(&state_mutex);
	return result;
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_sleepNative(void)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native sleepNative()");
	int result = 0;
	pthread_mutex_lock(&state_mutex);
	if(state == STATE_PAUSED)
	{
		state_req = STATE_SLEEPING;
		pthread_cond_wait(&state_cond_var,&state_mutex);
		result = 1;
	}
	pthread_mutex_unlock(&state_mutex);
	return result;
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_wakeNative(void)
{
	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native wakeNative()");
	int result = 0;
	pthread_mutex_lock(&state_mutex);
	if(state == STATE_SLEEPING)
	{
		pthread_mutex_lock(&sleep_mutex);
		state_req = STATE_PAUSED;
		pthread_cond_signal(&sleep_cond_var);
		pthread_mutex_unlock(&sleep_mutex);

		pthread_cond_wait(&state_cond_var,&state_mutex);
		result = 1;
	}
	pthread_mutex_unlock(&state_mutex);
	return result;
}
JNIEXPORT jint JNICALL Java_org_rin_RinThread_stopNative(JNIEnv *envs, jobject this)
{
	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native stopNative()");
	int result = 0;
	pthread_mutex_lock(&state_mutex);
	if(state == STATE_PAUSED && ! exited)
	{
		state_req = STATE_EXITED;
		//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native stopNative() - waiting for gamethread exit");
		pthread_cond_wait(&state_cond_var,&state_mutex);
		result = 3;
	}
	pthread_mutex_unlock(&state_mutex);
	
	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native stopNative() - exiting");
	return result;
}
JNIEXPORT jobject JNICALL Java_org_rin_RinService_getAudio(void)
{
	if(Audio)
		return Audio;
	else
		return NULL;
}
JNIEXPORT jobject JNICALL Java_org_rin_RinService_getRinThread(void)
{
	if(RinThread)
		return RinThread;
	else
		return NULL;
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_init(JNIEnv *envs, jobject this)
{
  	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native init()");
	RinThread_class = (*envs)->NewGlobalRef(envs,(*envs)->FindClass(envs, "org/rin/RinThread"));
	jmethodID constructor = (*envs)->GetMethodID(envs, RinThread_class, "<init>", "()V");
	RinThread = (*envs)->NewGlobalRef(envs,(*envs)->NewObject(envs, RinThread_class, constructor));

	pgaInit(envs);

	return 1;
}
JNIEXPORT jboolean JNICALL Java_org_rin_RinService_romGetLoaded(JNIEnv *envs, jobject this)
{
	return rom_get_loaded();
}
JNIEXPORT jint JNICALL Java_org_rin_RinService_romGetType(JNIEnv *envs, jobject this)
{
	return now_gb_mode;
}
JNIEXPORT jint JNICALL Java_org_rin_SpeedDialog_getSpeed(JNIEnv *envs, jobject this)
{
	return get_speed();
}
JNIEXPORT void JNICALL Java_org_rin_SpeedDialog_setSpeed(JNIEnv *envs, jobject this, jint speed)
{
	set_speed(speed);
}
JNIEXPORT jboolean JNICALL Java_org_rin_RinService_getTurbo(JNIEnv *envs, jobject this)
{
	if(bTurbo)
	  return 1;
	else 
	  return 0;
}
JNIEXPORT void JNICALL Java_org_rin_RinService_toogleTurbo(JNIEnv *envs, jobject this)
{
	if(bTurbo)
	  bTurbo = 0;
	else 
	  bTurbo = 1;
}
JNIEXPORT jboolean JNICALL Java_org_rin_RinService_getShowFPS(JNIEnv *envs, jobject this)
{
	if(setting2.show_fps)
	  return 1;
	else 
	  return 0;
}
JNIEXPORT void JNICALL Java_org_rin_RinService_toogleShowFPS(JNIEnv *envs, jobject this)
{
	if(setting2.show_fps)
	  setting2.show_fps = 0;
	else 
	   setting2.show_fps = 1;
}
JNIEXPORT jint JNICALL Java_org_rin_AutosaveDialog_getAutosave(JNIEnv *envs, jobject this)
{
	return setting.autosave;
}
JNIEXPORT void JNICALL Java_org_rin_AutosaveDialog_setAutosave(JNIEnv *envs, jobject this, jint intervall)
{
	setting.autosave = intervall; 
}
JNIEXPORT void JNICALL Java_org_rin_RinService_setAutosavePath(JNIEnv *envs, jobject this, jstring path)
{
  	char * ptmp = (char *) (*envs)->GetStringUTFChars(envs, path, 0);
	strcpy(AutosavePath,ptmp);
	(*envs)->ReleaseStringUTFChars(envs, path, ptmp);
}
JNIEXPORT void JNICALL Java_org_rin_RinService_setSramPath(JNIEnv *envs, jobject this, jstring path)
{
  	char * ptmp = (char *) (*envs)->GetStringUTFChars(envs, path, 0);
	strcpy(SramPath,ptmp);
	(*envs)->ReleaseStringUTFChars(envs, path, ptmp);
}
JNIEXPORT jboolean JNICALL Java_org_rin_RinService_getAutoCopyRom(JNIEnv *envs, jobject this)
{
	if(setting2.autocopy_rom)
	  return 1;
	else 
	  return 0;
}
JNIEXPORT void JNICALL Java_org_rin_RinService_toogleAutoCopyRom(JNIEnv *envs, jobject this)
{
	if(setting2.autocopy_rom)
	  setting2.autocopy_rom = 0;
	else 
	   setting2.autocopy_rom= 1;
}
JNIEXPORT jint JNICALL Java_org_rin_RinThread_startNative(JNIEnv *envs, jobject this, jstring rpath)
{
	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native start before mainloop");
	
	gettimeofday(&StartTime, NULL);

	char * ptmp = (char *) (*envs)->GetStringUTFChars(envs, rpath, 0);
	strcpy(RinPath,ptmp);
	(*envs)->ReleaseStringUTFChars(envs, rpath, ptmp);

	env_game_thread = envs;

	char tmp[PATH_MAX];
	
#ifdef PROFILE
	sprintf(tmp, "%sprof.txt", RinPath);
	pFile = fopen(tmp,"w");
	if(!pFile)
	{
	   //__android_log_print(ANDROID_LOG_ERROR, "org.rin", "filed to open file for storing profile information (path: %s)",tmp);
	}
	
	prof_init();
#endif
	
	
	order_queue = create_queue();
	
	if(!order_queue)
	{
		//__android_log_print(ANDROID_LOG_ERROR, "org.rin", "orror creating order_queue");
		return 0;
	}
	//else
	//	__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "success creating order_queue");


	state = STATE_PAUSED;
	state_req = 0;

	init_config();
	init_config_glob();
	
	
	//allocate memory for gameboy framebuffer VFRAME_SIZE plus some overflow buffer?
	word * gfpointer = (word *) malloc(VFRAME_SIZE + (160*5)*2 );
	gb_set_fp(gfpointer);
		
	gb_init();

	running = 1;
	exited = 0;

	mainloop();
	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native after mainloop");
	gb_destroy();

	free_rewind_states();

	delete_queue(order_queue);

	
#ifdef PROFILE
	if(pFile)
	{
		prof_print();
		fclose(pFile);
	}
#endif

	(*envs)->DeleteGlobalRef(envs,RinThread_class);
	(*envs)->DeleteGlobalRef(envs,RinThread);
	
	pthread_mutex_lock(&state_mutex);
	exited = 1;
	pthread_cond_signal( &state_cond_var );
	pthread_mutex_unlock( &state_mutex );
	
	free(gfpointer);

	//android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native before return to hell");

	return 4;
}
