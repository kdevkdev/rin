#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <pthread.h>
#include <sched.h>
#include <android/log.h>

#include "inc/main.h"
#include "font.c"


long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return (char *) pixels+x*dpixel_size*2 + y*dline_size*2;
}

void pgPrint_drawbg(unsigned long x, unsigned long y, unsigned long color, unsigned long bgcolor, const char *str)
{
	while (*str!=0 && x<dcmax_x && y<dcmax_y) {
		pgPutChar(x*8,y*8,color,bgcolor,*str,1,1,1);
		str++;
		x++;
		if (x>=dcmax_x) {
			x=0;
			y++;
		}
	}
}

void pgPrintf(unsigned long x,unsigned long y,unsigned long color,const char *str, ...)
{
	va_list ap;
	char szBuf[512];

	va_start(ap, str);
	vsprintf(szBuf, str, ap);
	va_end(ap);

	pgPrint2(x,y,color,szBuf);
}
void pgPrintfI(unsigned long x,unsigned long y,const char *str, ...)
{
	va_list ap;
	char szBuf[512];

	va_start(ap, str);
	vsprintf(szBuf, str, ap);
	va_end(ap);

	pgPrint2I(x,y,szBuf);
}
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<dcmax_x && y<dcmax_y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=dcmax_x) {
			x=0;
			y++;
		}
	}
}

void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<dcmax2_x && y<dcmax2_y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=dcmax2_x) {
			x=0;
			y++;
		}
	}
}
void pgPrint2I(unsigned long x,unsigned long y,const char *str)
{
	while (*str!=0 && x<dcmax2_x && y<dcmax2_y) {
		pgPutChar(x*16,y*16,0,0,*str,5,0,2);
		str++;
		x++;
		if (x>=dcmax2_x) {
			x=0;
			y++;
		}
	}
}



void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<dcmax4_x && y<dcmax4_y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=dcmax4_x) {
			x=0;
			y++;
		}
	}
}

/*void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=(unsigned char *)pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*dpixel_size + y1*dline_size] = color;
		((unsigned short *)vptr0)[i*dpixel_size + y2*dline_size] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*dpixel_size + i*dline_size] = color;
		((unsigned short *)vptr0)[x2*dpixel_size + i*dline_size] = color;
	}
}*/

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=(unsigned char *)pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*dpixel_size + i*dline_size] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=(unsigned char *)pgGetVramAddr(0,0);
	for (i=0; i<dframesize/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=dpixel_size*2;
	}
}

/*void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	unsigned short *dd;

	vptr0=(unsigned char *)pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=dpixel_size*2;
				}
				dd++;
			}
			vptr0+=dline_size*2;
		}
		d+=w;
	}

}*/

void pgBitBlitBilinearT(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short  *d)
{
#ifdef PROFILE
	prof_enter(PF_BIG_BLIT_BILIN);
#endif

	//we have 32 bits available, and assume resolutions of maximal 4096=2^12, so 20 bits are unused
	//shift numebers by 20 bit which should be large enough for precise pixel positioning and weighting calculations
	unsigned short *vptr = (unsigned short*)pgGetVramAddr(0, 0);
	unsigned short *vptrl = vptr;

	d += GGUARD_LINE;
	unsigned short*dl = d;

	//current traversal position in the image
	unsigned int cx =0;
	unsigned int cy =0;


	//increase value for the traversal position (shifted)
	unsigned int stepx = (dw << 20) / w;
	unsigned int stepy = (dh << 20) / h;

	//the four pixels to interpolate from and temporar pixels to save results
	unsigned short  tl, tr, bl, br;
	unsigned short  ti, bi;

	//weight factors
	 int wx=0;
	 int wy=0;
	 int wxo=0;;
	 int wyo=0;

	//weight steps (1/stepx = 1/(dw/w)=w/dw, use this to improve precision
	unsigned int wsx = (w<<20)/dw;
	unsigned int wsy = (h<<20)/dh;

	int o = 0;

	//run throug all the pixels that dont have the four nearest neighbours lapping over the borders
	wy = 0;
	wyo = 1<<20;


	for(cy = 0; cy < (dh<<20); cy+= (1<<20))
	{

		wx = 0;
		wxo = 1<<20;
		vptr = vptrl;
		d = dl;

		tl = d[0];
		tr = d[1];
		bl = d[GLINE_SIZE];
		br = d[GLINE_SIZE+1];

		for(cx = 0; cx < (dw<<20); cx+= 1 << 20)
		{
			//fetch the pixels (make assumption that the four neighbouring pixels have an distance of one in the index
			// o = (((cy)/stepy))*GLINE_SIZE + (((cx)/stepx));		



			//interpolate pixels
			ti = RGBA_FROMI((VREDA_8(tl)*wxo  + VREDA_8(tr)*wx),(VGREENA_8(tl)*wxo  + VGREENA_8(tr)*wx),(VBLUEA_8(tl)*wxo  + VBLUEA_8(tr)*wx));
			bi = RGBA_FROMI((VREDA_8(bl)*wxo  + VREDA_8(br)*wx),(VGREENA_8(bl)*wxo  + VGREENA_8(br)*wx),(VBLUEA_8(bl)*wxo  + VBLUEA_8(br)*wx));

			*vptr = RGBA_FROMI((VREDA_8(ti)*wyo  + VREDA_8(bi)*wy),(VGREENA_8(ti)*wyo  + VGREENA_8(bi)*wy),(VBLUEA_8(ti)*wyo  + VBLUEA_8(bi)*wy));

			//move one pixel forward in the destination image
			vptr++;

			//update weight factor
			wx += wsx;
			wxo -= wsx;

			if(wx > 1<<20)
			{
				wx -= (1<<20);
				wxo = (1 << 20) - wx;
				d++;

				tl = tr;
				bl = br;
				tr = d[1];
				br = d[GLINE_SIZE + 1];
			}

		}
		//update weight factor
		wy += wsy;
		wyo  -= wsy;
		if(wy > 1<<20)
		{
			wy -= (1<<20);
			wyo = (1 << 20)-wy;
			dl += GLINE_SIZE;
		}

		vptrl+= dline_size;
	}
#ifdef PROFILE
	prof_out();
#endif
}
void pgBitBlitBresham(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short * d)
{
#ifdef PROFILE
    //seems to work only for exact multiples of destination pixels / source pixels
    prof_enter(PF_BIG_BLIT_BRESHAM);
#endif
    
    
    d += GGUARD_LINE;
    
    unsigned short *vptr = (unsigned short*)pgGetVramAddr(0, 0);
    unsigned short *vptrl = vptr;
    unsigned short *dl = d;
    
    int rx = (dw << 10) / w;
    int ry = (dh << 10 )/ h;
    
    int cx = 0;
    int cy = 0;
    int sx =0;
    int sy=0;

    
    while(sy < h)
    {
	cy += ry;
	while(cy > ( 1 << 9))
	{
	    d=dl;
	    vptr  = vptrl; 
	    cx =0;
	    while(vptr < vptrl + dw)
	    {
		  cx += rx;
		  while(cx > (1<<9))
		  {
		      *vptr = *d;
		      vptr++;
		      cx -= 1<<10;
		  }
		  d++;
	    }
	    vptrl+= dw;
	    cy -= 1<<10;
	}
	dl+=GLINE_SIZE;
	sy++;
    }
   
#ifdef PROFILE
    prof_out();
#endif
}
void pgBitBlitBilinGrid(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, unsigned short  *d)
{
	
#ifdef PROFILE
	prof_enter(PF_BIG_BLIT_BILIN_GRID);
#endif	
	
	//idea: use bilinear interpolation only on where a destination pixel actually covers a source pixel
	unsigned short *vptr = (unsigned short*)pgGetVramAddr(0, 0);
	unsigned short *vptrl = vptr;

	d += GGUARD_LINE;
	unsigned short*dl = d;
	unsigned short *dend = d + (GSCREEN_HEIGHT) * GLINE_SIZE;

	int l,r, t, b, tl, tr, bl, br, cxo, cyo;
	#define  SA 20
	int cx = 0;
	int cy = 0;
	//again keep it integer only
	int x_target_pixel_size = 1 << SA; 
	int x_source_pixel_size = (dw << SA)/ w;

	int y_target_pixel_size = 1 << SA; 
	int y_source_pixel_size = (dh << SA)/ h;

	while(d < dend)
	{
		for(cy += y_source_pixel_size; cy >= y_target_pixel_size; cy -= y_target_pixel_size)
		{
			//in this loop we only need to look at one line in y direction - no bilin interpolation
			cx = 0;
			vptrl = vptr;
			for(dl = d; dl < d + GSCREEN_WIDTH; dl++)
			{
				for(cx += x_source_pixel_size; cx >= x_target_pixel_size; cx-= x_target_pixel_size)
				{
					//just convert colors, no bilinear interpolation
					*vptrl = *dl;
					vptrl ++;
				}
				//do only here bilin interpolation
				if(cx > 0)
				{
					cxo = x_target_pixel_size - cx;
					l = *dl;
					r = *(dl+1);
					*vptrl =  RGBA_FROMI((VREDA_8(l)*cx  + VREDA_8(r)*cxo),(VGREENA_8(l)*cx  + VGREENA_8(r)*cxo),(VBLUEA_8(l)*cx  + VBLUEA_8(r)*cxo));
					cx-= x_target_pixel_size;					
					vptrl++;
				}
			}
			vptr += dline_size;
		}

		//do bilinear interpolation also in y direction
		if(cy > 0)
		{
			cx = 0;
			cyo = y_target_pixel_size - cy;
			vptrl = vptr;
			for(dl = d; dl < d + GSCREEN_WIDTH; dl++)
			{
				for(cx += x_source_pixel_size; cx >= x_target_pixel_size; cx-= x_target_pixel_size)
				{
					//only interpolate in y direction
					t = *dl;
					b = *(dl+GLINE_SIZE);
					*vptrl =  RGBA_FROMI((VREDA_8(t)*cy  + VREDA_8(b)*cyo),(VGREENA_8(t)*cy  + VGREENA_8(b)*cyo),(VBLUEA_8(t)*cy  + VBLUEA_8(b)*cyo));
					vptrl ++;
				}
				if(cx > 0)
				{
					//do both interpolate in y and x direction
					cxo = x_target_pixel_size - cx;

					tl = *dl;
					tr = *(dl+1);
					t =  RGBA_FROMI((VREDA_8(tl)*cx  + VREDA_8(tr)*cxo),(VGREENA_8(tl)*cx  + VGREENA_8(tr)*cxo),(VBLUEA_8(tl)*cx  + VBLUEA_8(tr)*cxo));

					bl = *(dl+GLINE_SIZE);
					br = *(dl+GLINE_SIZE+1);
					b =  RGBA_FROMI((VREDA_8(bl)*cx  + VREDA_8(br)*cxo),(VGREENA_8(bl)*cx  + VGREENA_8(br)*cxo),(VBLUEA_8(bl)*cx  + VBLUEA_8(br)*cxo));

					cx-= x_target_pixel_size;
					*vptrl =  RGBA_FROMI((VREDA_8(t)*cy  + VREDA_8(b)*cyo),(VGREENA_8(t)*cy  + VGREENA_8(b)*cyo),(VBLUEA_8(t)*cy  + VBLUEA_8(b)*cyo));
					vptrl++;
				}
			}
			cy-= y_target_pixel_size;
			vptr += dline_size;
		}

		//advance the gameboy framepointer by one line
		d += GLINE_SIZE;
	}

#ifdef PROFILE	
	prof_out();
#endif
}
void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	cfont=font+ch*8;
	vptr0=(unsigned char *)pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) 
				{
					if (((*cfont) &b)!=0) 
					{

						if(drawfg == 5)
						{
							*(unsigned short *)vptr= 0xFFFF -*vptr;
							//*(unsigned short *)vptr= RGB_A(255 - VREDA_8(*vptr), 255- VGREENA_8(*vptr), 255 - VBLUEA_8(*vptr));
						}
						else if (drawfg) 
						{
							*(unsigned short *)vptr=color;
						}
					}
					else 
					{
						if (drawbg) 
						{
							*(unsigned short *)vptr=bgcolor;
						}
					}
					vptr+=dpixel_size*2;
				}
				b=b>>1;
			}
			vptr0+=dline_size*2;
		}
		cfont++;
	}
}
void pgScreenFlip()
{
#ifdef PROFILE
	prof_enter(PF_PG_SCREEN_FLIP);
#endif

	if(edrawing)
	{
		AndroidBitmap_unlockPixels(env_game_thread,buf_bitmap);
		(*env_game_thread)->CallVoidMethod(env_game_thread,View,doDrawSingle);	
		AndroidBitmap_lockPixels(env_game_thread,buf_bitmap, &pixels);
		//gb_set_fp((word *) pixels);
	}
	
#ifdef PROFILE
	prof_out();
#endif
}

/******************************************************************************/
int pgaInit(JNIEnv *envs)
{
	sound_buf_len = SOUND_BUF_LEN;
	sound_buf = malloc(SOUND_BUF_LEN*(BANKS)*sizeof(short));
	__memset4(sound_buf, 0,(sound_buf_len*BANKS)/2);


	//start Audio-Thread;
	RinAudio_class = (*envs)->NewGlobalRef(envs,(*envs)->FindClass(envs, "org/rin/RinAudio"));
	jmethodID constructor = (*envs)->GetMethodID(envs, RinAudio_class, "<init>", "(I)V");
	Audio = (*envs)->NewGlobalRef(envs,(*envs)->NewObject(envs, RinAudio_class, constructor, (jint) sound_buf_len));

	return 1;
}
JNIEXPORT void JNICALL Java_org_rin_RinAudioRunnable_pgaTerm(JNIEnv *envt, jobject this)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native pgaTerm");
	(*envt)->DeleteGlobalRef(envt,RinAudio_class);
	(*envt)->DeleteGlobalRef(envt,Audio);
	free(sound_buf);
}
short textcolor[5] = {RGBA_8(255,255,255),RGBA_8(0,0,0),RGBA_8(255,20,20),RGBA_8(20,255,20),RGBA_8(20,20,255)};

//orientation: 0 -- unspecified, 1 -- portrait, 2 -- landscape
JNIEXPORT jint JNICALL Java_org_rin_RinView_pgInit(JNIEnv *envt,jobject this, jobject drawobj, jobject bitmap, jint tr_x, jint tr_y, jint orientation)
{
  	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native pgInit");

	//need to make those global because local references can be garbage collected after the native method returns and and it will be used in different thread
	View =(*envt)->NewGlobalRef(envt,drawobj);
	buf_bitmap = (*envt)->NewGlobalRef(envt,bitmap);

	RinView_class = (*envt)->NewGlobalRef(envt,(*envt)->GetObjectClass(envt, drawobj));
	doDrawSingle = (*envt)->GetMethodID(envt,RinView_class,"doDrawSingle","()V");
	
	edrawing = 0;


	dpixel_size = 1;
	dline_size = tr_x;
	dscreen_width = tr_x;
	dscreen_height = tr_y;

	//for font rendering
	dcmax_x = dscreen_width / 8;
	dcmax_y = dscreen_height / 8;
	dcmax2_x = dscreen_width / 16;
	dcmax2_y = dscreen_height / 16;
	dcmax4_x = dscreen_width / 32;
	dcmax4_y = dscreen_height / 32;
	
	
	
	dfps_posx = dcmax2_x -5;
	dfps_posy = dcmax2_y -2;


	//for bilinear Interpolation -- let the preprocessor caculate them
	//x and y pixel weight steps (NOTE! when changing the screen sizes, its possible that an overflow occurs, because these weight steps 
	//are only 15 bits plus sign wide)
	//with those screen sizes it is best for max precision to store the source screen size in 8 bits and shift by 7
	dbilin_wsx = ((GSCREEN_WIDTH  << SHIFT_AM) / dscreen_width);
	dbilin_wsy = ((GSCREEN_HEIGHT  << SHIFT_AM) / dscreen_height);

	dcomb_wx =  ((-dbilin_wsx) & 0x0000FFFF)+(dbilin_wsx<<16);
	dcomb_wy =  ((-dbilin_wsy) & 0x0000FFFF)+(dbilin_wsy<<16);

 	dframesize = dline_size * dscreen_height*2;
	
	AndroidBitmapInfo  info;
	if ((AndroidBitmap_getInfo(envt, buf_bitmap, &info)) < 0) {
		//invalid bitmap
		return -1;
	}

	if (info.format != ANDROID_BITMAP_FORMAT_RGB_565) {
		//wrong format
		return -1;
        }



	if(AndroidBitmap_lockPixels(envt, buf_bitmap, &pixels) < 0) return -1;
	
	
	//gb_set_fp((word *) pixels);
	
	pg_showframe = 1;
	pg_drawframe = 0;

	return 1;
}
JNIEXPORT jint JNICALL Java_org_rin_RinView_pgTerm(JNIEnv *envt, jobject this)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native pgTerm");
	//counterpart to pgiInit, so everything allocated there needs to be freed here
	(*envt)->DeleteGlobalRef(envt,RinView_class);
	(*envt)->DeleteGlobalRef(envt,buf_bitmap);
	(*envt)->DeleteGlobalRef(envt,View);
}
JNIEXPORT void JNICALL Java_org_rin_RinService_enableDrawing(void)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native enableDrawing");
	renderer_init();
	edrawing = 1;
}
JNIEXPORT void JNICALL Java_org_rin_RinService_disableDrawing(void)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "native disableDrawing");
	edrawing = 0;
}
JNIEXPORT jint JNICALL Java_org_rin_ScreenDialog_getScaleMethod(JNIEnv *envs, jobject this)
{
    if(setting2.scale_method < SCALE_METHOD_END)
      return setting2.scale_method;
    else 
      return SCALE_METHOD_BILINEAR_GRID;
}
JNIEXPORT void JNICALL Java_org_rin_ScreenDialog_setScaleMethod(JNIEnv *envs, jobject this,jint id)
{
    if(id < SCALE_METHOD_END)
	setting2.scale_method = id;
}