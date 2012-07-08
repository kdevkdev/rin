#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <jni.h>
#include <pthread.h>
#include <android/bitmap.h>
//#include <android/log.h>
#include <sys/limits.h>

#include "zlib.h"
#include "queue.h"

#include "pgshared.h"
#include "pg.h"
#include "graphics.h"
#include "conf.h"
#include "../gb/gb.h"
#include "order.h"

#include "sound.h"
#include "filer.h"
#include "saveload.h"

//profile functions - if you add some more you should also put the according string into rin.c in prof_init() - values up to 256 possible
#define PF_GB_RUN 0
#define PF_RENDER_SCREEN 1
#define PF_BIG_BLIT_STFULL 2
#define PF_BIG_BLIT_BILIN 21
#define PF_BIG_BLIT_BILIN_GRID 22
#define PF_BIG_BLIT_BRESHAM 23
#define PF_RENDERER_UPDATE_SOUND 3
#define PF_PG_SCREEN_FLIP 4
#define PF_CPU_EXEC 5

extern char RinPath[], RomPath[], SavePath[], RomName[], CheatPath[], ScreenPath[];
extern int bMenu, bSleep, bPausek, state, state_req, rin_rewind;
extern queue_t * order_queue;

unsigned int sync_time;
unsigned int slow_mod;


#endif