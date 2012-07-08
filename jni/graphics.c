#include "inc/main.h"
#include "inc/lodepng.h"

static int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

/*void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}*/
void saveImage(const char* filename, short * data, int width, int height, int lineSize, int saveAlpha)
{
	uint8_t * buf = malloc(width * height * 4);
	
	uint8_t *bs = buf;
	
	short * dl = data;
	int i=0,j=0;
	for(i =0; i < height; i++)
	{
		data = dl;
		for(j =0; j < width; j++)
		{
			*bs = VREDA_8(*data); 
			bs++;
			*bs = VGREENA_8(*data);
			bs++;
			*bs = VBLUEA_8(*data);
			bs++;
			*bs = saveAlpha ? ((*data) >> 24) & 0xff : 0xff;
			bs++;
			data++;
		}
		dl+= lineSize;
	}
	
	 /*Encode the image*/
	unsigned error = lodepng_encode32_file(filename, buf, width, height);
	
	free(buf);
}
/*Image* loadImage(char* filename)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	u32* line;
	FILE *fp;
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;

	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		free(image);
		fclose(fp);
		return NULL;;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	if (width > 512 || height > 512) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	line = (u32*) malloc(width * 4);
	if (!line) {
		free(image->data);
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, NULL);
		for (x = 0; x < width; x++) {
			u32 color = line[x];
			image->data[x + y * image->textureWidth] =  color;
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return image;
}*/


