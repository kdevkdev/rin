// lcd.c (gameboy buffer dimension)
#define GLINE_SIZE    176		//in pixel
#define GCOLUMN_SIZE 144+112	//in pixel
#define GGUARD_LINE   8		//in pixel

//gameboy screen dimensions
#define GSCREEN_WIDTH 160
#define GSCREEN_HEIGHT 144


//pg.c
#define	LINE_SIZE 	480
#define SCREEN_WIDTH  	480
#define SCREEN_HEIGHT 	432
#define	PIXEL_SIZE	1


//for bilinear Interpolation -- shift amount
#define SHIFT_AM 7

#define FIX_ONE (1 << SHIFT_AM) //needs to be arm friendly


#define MSG "gggooododo %d:%d \n"
