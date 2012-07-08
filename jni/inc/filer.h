#ifndef FILER_H
#define FILER_H

#include <dirent.h> 

long load_rom(const char *szRomPath);
int getFilePath(char *fullpath, u32 ext);

// —LŒø‚ÈŠg’£Žq
enum {
	EXT_GB = 1,
	EXT_GZ = 2,
	EXT_ZIP = 4,
	EXT_TCH = 8,
	EXT_IMG = 16,
	EXT_TXT = 32,
	EXT_UNKNOWN = 64,
};

#endif
