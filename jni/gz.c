
#include "inc/gz.h"

int gzGetOrigName(const char *path, char *out, gzFile fd)
{
	if (!fd)
		return 0;

	if (((gz_stream*)fd)->transparent == 1)
		return 0;

	if (((gz_stream*)fd)->orig_name[0] == 0){
		strcpy(out, strrchr(path,'/')+1);
		char *p = strrchr(out, '.');
		if (p && !strcmp(p, ".gz")){
			*p = 0;
			return 1;
		}
		return 0;
	}

	strcpy(out, ((gz_stream*)fd)->orig_name);

	return 1;
}

int gzCreate(FILE * fd, unsigned char *inbuf, uint32_t inlen)
{
	const uint8_t header[] = {0x1f,0x8b,Z_DEFLATED,0,0,0,0,0,0,OS_CODE};

	uint32_t outlen = ((inlen+12)/100+1)*101;
	uint8_t *outbuf = (uint8_t*)malloc(outlen+sizeof(header)+sizeof(uint32_t)*2);
	if (!outbuf) return 0;

	z_stream s;
    	s.zalloc = (alloc_func)0;
    	s.zfree = (free_func)0;
    	s.opaque = (voidpf)0;
    	s.next_in = Z_NULL;
    	s.next_out = Z_NULL;
    	s.avail_in = s.avail_out = 0;

	int err;
	err = deflateInit2(&s, Z_DEFAULT_COMPRESSION,
		Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	if (err!=Z_OK){
		free(outbuf);
		return 0;
	}

	s.avail_in = inlen;
	s.avail_out = outlen;
	s.next_in = inbuf;
	s.next_out = outbuf+sizeof(header);

	err = deflate(&s, Z_FINISH);
	if (err != Z_STREAM_END){
		err = deflateEnd(&s);
		free(outbuf);
		return 0;
	}
	err = deflateEnd(&s);

	uint32_t size = outlen - s.avail_out;
	uint32_t crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, inbuf, inlen);

	memcpy(outbuf, header, sizeof(header));
	memcpy(outbuf+sizeof(header)+size, &crc, sizeof(uint32_t));
	memcpy(outbuf+sizeof(header)+size+sizeof(uint32_t), &inlen, sizeof(uint32_t));

	int writesize = fwrite(outbuf, size+sizeof(header)+sizeof(uint32_t),2,fd);
	free(outbuf);

	if (writesize != 2)
		return 0;
	else
		return 1;
}

