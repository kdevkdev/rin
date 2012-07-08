#ifndef PG_H
#define PG_H
//partly reworked primitive graphics library for psp

#include <jni.h>


#define NUM2HEX(n) ((n) & 0xFF)

//USED PIXELFORMAT IN ANDROID IS 565
#define RGBA_8(r,g,b) ((((r>>3) & 0x1F)<<11)|(((g>>2) & 0x3F)<<5)|(((b>>3) & 0x1F)<<0))
#define RGBA(r,g,b) ((((r) & 0x1F)<<11)|(((g) & 0x3F)<<5)|(((b) & 0x1F)<<0))

//From integer math format (left shift by 20) directly  to android format -- values expected to be one byte wide (from 0 to 255)
#define RGBA_FROMI(r,g,b) (((r >> 12)& 0xF800) | ((g  >> 17)& 0x07E0) | ((b >> 23)))


//#define RGB_555_TO_565(c) (RGB_A(VREDG_8(c),VGREENG_8(c),VBLUEG_8(c)))
//#define RGB_555_TO_565(c) ((c << 1) &  0xFFC0) |  (c & 0x1F)
#define RGB_555_TO_565(c) ((((c << 11) &  0xF800) |  ((c >> 10) & 0x1F ))  | ((c << 1) & 0x7C0)) //red and blue need to be swapped

#define RGB_565_TO_555(c) ((((c >> 11) &  0x1F  ) |  ((c << 10) & 0x7C00)) | ((c >> 1) & 0x3E0))


//bitmasks and shifts to retrieve color values of android format  --r g b, b g r
#define VREDA(c)  ((c & 0xF800) >> 11)
#define VGREENA(c) ((c & 0x07E0) >> 5)
#define VBLUEA(c) (c & 0x001F)

#define VREDA_8(c) (VREDA(c) << 3)
#define VGREENA_8(c) (VGREENA(c) << 2)
#define VBLUEA_8(c) (VBLUEA(c) << 3)

//bitmasks and shifts to retrieve color values of gameboy format
#define VREDG(c)  ((c >> 0) & 0x01F)
#define VGREENG(c) ((c >> 5) & 0x01F)
#define VBLUEG(c) ((c >> 10) & 0x01F)

//to byte
#define VREDG_8(c) (VREDG(c) << 3)
#define VGREENG_8(c) (VGREENG(c) << 3)
#define VBLUEG_8(c) (VBLUEG(c) << 3)

//only the masks
#define MRED(c)  ((c & 0xF800))
#define MGREEN(g) ((c & 0x7E0))
#define MBLUE(b) (c & 0x1F)


//to integer math format
#define IRED(c)  ((c & 0xF800) << 9)
#define IGREEN(g) ((c & 0x7E0) << 15)
#define IBLUE(b) (c & 0x1F)    << 20)


//scale methods:
#define SCALE_METHOD_BRESHAM 0
#define SCALE_METHOD_BILINEAR_GRID 1
#define SCALE_METHOD_BILINEAR 2
#define SCALE_METHOD_END 3


extern short textcolor[5];
extern uint32_t new_pad, now_pad, old_pad;
jobject buf_bitmap, Audio, View, RinThread;
jmethodID doDrawSingle;
void * pixels;
jclass RinAudio_class, RinView_class, RinThread_class;
JNIEnv * env_game_thread;

int edrawing;

//in samples
int sound_buf_len;
short * sound_buf;


typedef int32_t Color;
#define A(color) ((uint8_t)(color >> 24 & 0xFF))
#define B(color) ((uint8_t)(color >> 16 & 0xFF))
#define G(color) ((uint8_t)(color >> 8 & 0xFF))
#define R(color) ((uint8_t)(color & 0xFF))

//global variable version of screen dimension variables for use with dynamic resolution
int dpixel_size;
int dline_size;
int dscreen_width;
int dscreen_height;
int dbilin_wsx;
int dbilin_wsy;
int dcomb_wx;
int dcomb_wy;
int dframesize;
int dcmax_x, dcmax_y, dcmax2_x, dcmax2_y, dcmax4_x, dcmax4_y;
int dfps_posx, dfps_posy;
int draw_fps;




JNIEXPORT jobject JNICALL Java_org_rin_rin_getAudio(void);

void pgScreenFlip();
void pgPrintf(unsigned long x,unsigned long y,unsigned long color,const char *str, ...);
void pgPrintfI(unsigned long x,unsigned long y,const char *str, ...);
void pgPrint_drawbg(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,const char *str);
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint2(unsigned long x,unsigned long y ,unsigned long color,const char *str);
void pgPrint2I(unsigned long x,unsigned long y,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgFillvram(unsigned long color);

//void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,unsigned short *d);
//void pgBitBltStFull(unsigned long x,unsigned long y,unsigned long h,unsigned long mag,const unsigned short *d);

void pgBitBlitBilinear(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short  *d);
void pgBitBlitBilinearT(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short  *d);
void pgBitBlitBilinGrid(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short  *d);
extern void pgBitBlitBilinearS(unsigned short * d, unsigned short * vptr, int comb_wx, int comb_wy);


void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch, char drawfg,char drawbg,char mag);
//void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
char *pgGetVramAddr(unsigned long x,unsigned long y);


extern uint32_t now_tick;

//Todo: optimize for arm -- maybe thei are useful at a later point

/*//long配列をコピー。配列境界は4バイトアラインされている必要あり
static inline void __memcpy4(void *d, void *s, unsigned long c)
{
	//for (; c>0; --c) *(((unsigned long *)d)++)=*(((unsigned long *)s)++);
	unsigned long *dd=d, *ss=s;
	for (; c>0; --c) *dd++ = *ss++;
}*/


//long配列にセット。配列境界は4バイトアラインされている必要あり
static inline void __memset4(void *d, unsigned long v, unsigned long c)
{
	//for (; c>0; --c) *(((unsigned long *)d)++)=v;
	unsigned long *dd=d;
	for (; c>0; --c) *dd++=v;
}

#endif