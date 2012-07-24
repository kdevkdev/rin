#ifndef FILER_H
#define FILER_H

//#include <dirent.h> 


//javatized file io: provides easy gzip and zip support based on file extension with C- like interface
//needs to be synced with cfile.java
//also endianness needs to be considered - but i think the jvm does that already?
static int JF_MODE_NEW = 1;
static int JF_MODE_READ = 2;
static int JF_MODE_OVERWRITE = 4;
static int JF_MODE_WRITE = 8;


typedef jobject jfile;
//warning: those functions only work in one thread in one jni context
int 	jfread(JNIEnv * env, jfile cf, void * buffer, int length);
void 	jfclose(JNIEnv * env, jfile cf);
void 	jfwrite(JNIEnv * env, jfile cf, void * buffer, int length);
jfile jfopen(JNIEnv * env,char * path, int mode, char ** extract_types, int ntypes);
int jfsize(JNIEnv * env, jfile cf);

long load_rom(char *szRomPath);
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
