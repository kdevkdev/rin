#include "inc/main.h"
//#include "inc/gz.h"
//#include "inc/zlibInterface.h"
#include <dirent.h>

char path_inzip[PATH_MAX]={0};


// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
 "gb",EXT_GB,
 "gbc",EXT_GB,
 "sgb",EXT_GB,
 "gz",EXT_GZ,
 "zip",EXT_ZIP,
 "tch",EXT_TCH,
 "png", EXT_IMG,
 NULL, EXT_UNKNOWN
};
int getExtId(const char *szFilePath) {
	char *pszExt;

	if ((pszExt = strrchr(szFilePath, '.'))) {
		pszExt++;
		int i;
		for (i = 0; stExtentions[i].nExtId != EXT_UNKNOWN; i++) {
			if (!strcasecmp(stExtentions[i].szExt,pszExt)) {
				return stExtentions[i].nExtId;
			}
		}
	}

	return EXT_UNKNOWN;
}

// Unzip 対応 by ruka

// コールバック受け渡し用
typedef struct {
	byte *p_rom_image;			// pointer to rom image
	long rom_size;				// rom size
	char szFileName[PATH_MAX];		// extracted file name
}ROM_INFO, *LPROM_INFO;

// Unzip コールバック
/*int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData)
{
    const char *pszFileName;
    int nExtId;
    const unsigned char *pbData;
    LPROM_INFO pRomInfo = (LPROM_INFO)ulUserData;

    switch(nCallbackId) {
    case UZCB_FIND_FILE:
		pszFileName = (const char *)pData;
		
		if (path_inzip[0] && strcmp(pszFileName,path_inzip))
			break;
		
		nExtId = getExtId(pszFileName);
		// 拡張子がGB・GBC・SGBなら展開
		if (nExtId == EXT_GB) {
			// 展開する名前、rom sizeを覚えておく
			strcpy(pRomInfo->szFileName, pszFileName);
			pRomInfo->rom_size = ulExtractSize;
			return UZCBR_OK;
		}
        break;
    case UZCB_EXTRACT_PROGRESS:
		pbData = (const unsigned char *)pData;
		// 展開されたデータを格納しよう
		memcpy(pRomInfo->p_rom_image + ulCurrentPosition, pbData, ulDataSize);
		return UZCBR_OK;
        break;
    default: // unknown...
		pgFillvram(RGBA_8(255,0,0));
		pgPrintf(0,0,0xFFFF, "Unzip fatal error.");
		pgScreenFlip();
        break;
    }
    return UZCBR_PASS;
}*/

// load rom image by ruka
long load_rom(char *szRomPath)
{
	char name[MAX_NAME], *p, tmp[PATH_MAX];
	long lReadSize=0;
	
	p = strrchr(szRomPath, '/');
	if (!p)
		return 0;
	strcpy(name, p+1);
	char* exts[3]= {".gb",".gbc",".sgb"};
	
	
	jfile fd = jfopen(env_game_thread, szRomPath, JF_MODE_READ, exts, 3);
	
	if(fd)
	{
		int size = jfsize(env_game_thread, fd);
		lReadSize = jfread(env_game_thread, fd, rom_image, size);
		jfclose(env_game_thread, fd);
	}
	
	//lReadSize = jfile_read(env_game_thread, szRomPath, exts,3,(void **) &rom_image);
	
	/*ROM_INFO stRomInfo;
	int nRet;
	int nExtId = getExtId(szRomPath);

	switch(nExtId) {
	case EXT_GB:{	// "gb" "gbc" "sgb"
		p = strrchr(szRomPath, '/');
		if (!p)
			return 0;
		strcpy(name, p+1);

		FILE * fd = fopen(szRomPath, "r");
		lReadSize = fread(rom_image, 1, MAX_ROM_SIZE, fd);
		fclose(fd);
		break

	}case EXT_GZ:{	// "gz"
		gzFile fd = gzopen(szRomPath, "r");
		if (!gzGetOrigName(szRomPath, name, fd)){
			gzclose(fd);
			return 0;
		}else if (getExtId(name)!=EXT_GB){
			gzclose(fd);
			return 0;
		}
		lReadSize = gzread(fd, rom_image, MAX_ROM_SIZE);
		gzclose(fd);
		break;

	}case EXT_ZIP:	// "zip"
		if (path_inzip[0]){
			if (getExtId(path_inzip)!=EXT_GB)
				return 0;
			p = strrchr(path_inzip, '/');
			if (!p)
				p = path_inzip;
			else
				p++;
			strcpy(name, p);
		}else{
			p = strrchr(szRomPath, '/');
			if (!p)
				return 0;
			strcpy(name, p+1);
		}
			

		stRomInfo.p_rom_image = rom_image;
		stRomInfo.rom_size = 0;
		memset(stRomInfo.szFileName, 0x00, sizeof(stRomInfo.szFileName));
		// Unzipコールバックセット
		Unzip_setCallback(funcUnzipCallback);
		// Unzip展開する
	    nRet = Unzip_execExtract(szRomPath, (unsigned long)&stRomInfo);
		if (nRet != UZEXR_OK) {
			// 読み込み失敗！ - このコードでは、UZEXR_CANCELもここに来て
			// しまうがコールバックでキャンセルしてないので無視
			lReadSize = 0;
			pgFillvram(RGBA_8(255,0,0));
			pgPrintf(0,0,0xFFFF, "Unzip fatal error.");
			pgScreenFlip();
		}
		lReadSize = stRomInfo.rom_size;
		break;
	default:
		return 0;
	}*/
	
	if(lReadSize){
		strcpy(RomName, name);
		p = strrchr(RomName, '.');
		if(p)
			*p = 0;
	}
	
	return lReadSize;
}
/*int  jfile_read(JNIEnv * env, char * path, char ** extract_type, int ntypes, void ** dest)
{
	//first get utf string for filepath
	jstring jpath = (*env)->NewStringUTF(env, path);
	
	//convert the array to an java array, it contains a list of fil endings that are tested against the contents of the zip file, the first zip entry
	//to match one extension will be extracted into memory
	jclass string_class = (*env)->FindClass(env,"java/lang/String");
	jstring emptystring = (*env)->NewStringUTF(env,"");
	jobjectArray etypes = (jobjectArray) (*env)->NewObjectArray(env, ntypes, string_class, emptystring);
	
	int i;
	for(i=0; i< ntypes; i++)
	{
		        (*env)->SetObjectArrayElement(env,etypes,i,(*env)->NewStringUTF(env, extract_type[i]));
	}
	
	//get class, method id and create the FileProvider object
	jclass fp_class = (*env)->FindClass(env, "org/rin/FileProvider");
	jmethodID constructor = (*env)->GetMethodID(env, fp_class, "<init>", "([Ljava/lang/String;)V");
	jobject fileProvider = (*env)->NewObject(env, fp_class, constructor, etypes);
	
	//call the actual function
	jmethodID readFileUncompress = (*env)->GetMethodID(env, fp_class, "readFileUncompress", "(Ljava/lang/String;)[B");
	jbyteArray jbuffer = (*env)->CallObjectMethod(env,fileProvider,readFileUncompress,jpath);
	
	//reserver memory to copy the java array contents into it (requires twice the memory but correct access to java allocated memory can be quite hard)
	//so this should be sturdier but with the free wrapper function below it is possible to change this behaviour later and maybe prevent double memory usage
	//but only maybe since according to this http://www.ibm.com/developerworks/java/library/j-jni/ the jvm still could copy the array.
	int length = (*env)->GetArrayLength(env,jbuffer);
	*dest = malloc(length);
	(*env)->GetByteArrayRegion(env,jbuffer,0,length,*dest);
	
	
	//free stuff
	(*env)->DeleteLocalRef(env,jpath);
	(*env)->DeleteLocalRef(env,string_class);
	(*env)->DeleteLocalRef(env,emptystring);
	
	//delete evry array element individually
	for(i=0; i< ntypes; i++)
	{
		        (*env)->DeleteLocalRef(env,(*env)->GetObjectArrayElement(env,etypes,i));
	}
	
	(*env)->DeleteLocalRef(env, etypes);
	(*env)->DeleteLocalRef(env, fp_class);
	(*env)->DeleteLocalRef(env, fileProvider);
	(*env)->DeleteLocalRef(env, jbuffer);
	
	return length;
}
void jfile_free(JNIEnv * env, void * buffer)
{
	free(buffer);
}
void jfile_write(JNIEnv * env,char * path,void * buffer, int length)
{
	//first get utf string for filepath
	jstring jpath = (*env)->NewStringUTF(env, path);
	
	//get class, method id and create the FileProvider object
	jclass fp_class = (*env)->FindClass(env, "org/rin/FileProvider");
	jmethodID constructor = (*env)->GetMethodID(env, fp_class, "<init>", "([Ljava/lang/String;)V");
	jobject fileProvider = (*env)->NewObject(env, fp_class, constructor, NULL);
	
	//array that hold file contents
	jbyteArray jbuffer = (*env)->NewByteArray(env,length);
	(*env)->SetByteArrayRegion(env, jbuffer, 0,length, (byte *) buffer);
	
	//call the actual function
	jmethodID writeFileCompress = (*env)->GetMethodID(env, fp_class, "writeFileCompress", "([BLjava/lang/String;)V");
	(*env)->CallVoidMethod(env,fileProvider,writeFileCompress, jbuffer, jpath);
	
	//free stuff
	(*env)->DeleteLocalRef(env, jpath);
	(*env)->DeleteLocalRef(env, fp_class);
	(*env)->DeleteLocalRef(env, fileProvider);
	(*env)->DeleteLocalRef(env, jbuffer);
}*/

//file io wrappers through java, need those to provide easy zip and gzip support without a need to include zlip and some helper library
//for modes see filler.h
int jfsize(JNIEnv * env, jfile cf)
{
	jclass cf_class = (*env)->GetObjectClass(env,cf);
	jmethodID getSizeUncompressed = (*env)->GetMethodID(env, cf_class, "getSizeUncompressed", "()I");
	int size = (*env)->CallIntMethod(env, cf, getSizeUncompressed);
	
	(*env)->DeleteLocalRef(env, cf_class);
	
	return size;
}
jfile jfopen(JNIEnv * env,char * path, int mode, char ** extract_types, int ntypes)
{
	//first get utf string for filepath
	jstring jpath = (*env)->NewStringUTF(env, path);
	
	//fill in the endigs for the zip file
	jclass string_class = (*env)->FindClass(env,"java/lang/String");
	jstring emptystring = (*env)->NewStringUTF(env,"");
	jobjectArray etypes = (jobjectArray) (*env)->NewObjectArray(env, ntypes, string_class, emptystring);
	
	int i;
	for(i=0; i< ntypes; i++)
	{
	        (*env)->SetObjectArrayElement(env,etypes,i,(*env)->NewStringUTF(env, extract_types[i]));
	}
	
	//get class, method id and create the cFile object
	jclass cf_class = (*env)->FindClass(env, "org/rin/cFile");
	jmethodID constructor = (*env)->GetMethodID(env, cf_class, "<init>", "(Ljava/lang/String;I[Ljava/lang/String;)V");
	jobject cFile = (*env)->NewObject(env, cf_class, constructor, jpath, mode, etypes);
	
	jthrowable exc = (*env)->ExceptionOccurred(env);
	
	//check if file is ok
	jmethodID getOk = (*env)->GetMethodID(env, cf_class, "getOk", "()I");
	int ok = (*env)->CallIntMethod(env, cFile, getOk);
	
	if(exc)
	{		
		//something gone wrong -- return zero
		(*env)->ExceptionClear(env);
		(*env)->DeleteLocalRef(env,cFile);
		(*env)->DeleteLocalRef(env,exc);
		cFile = NULL;
	}
	else if(ok < 1)
	{
		(*env)->DeleteLocalRef(env,cFile);
		cFile = NULL;
	}
	
	//free stuff
	(*env)->DeleteLocalRef(env,jpath);
	(*env)->DeleteLocalRef(env,string_class);
	(*env)->DeleteLocalRef(env,emptystring);
	
	//delete evry array element individually
	for(i=0; i< ntypes; i++)
	{
		        (*env)->DeleteLocalRef(env,(*env)->GetObjectArrayElement(env,etypes,i));
	}
	
	(*env)->DeleteLocalRef(env, etypes);
	(*env)->DeleteLocalRef(env, cf_class);
	return cFile;
}
void jfwrite(JNIEnv * env, jobject cf, void * buffer, int length)
{
	//java Array to hold the buffer contents
	jbyteArray jbuffer = (*env)->NewByteArray(env,length);
	(*env)->SetByteArrayRegion(env, jbuffer, 0,length, (byte *) buffer);
	
	//get class
	jclass cf_class = (*env)->GetObjectClass(env,cf);
	
	//call the actual function
 	jmethodID write = (*env)->GetMethodID(env, cf_class, "write", "([BII)V");
	(*env)->CallVoidMethod(env,cf,write, jbuffer,0, length);
	
	(*env)->DeleteLocalRef(env, jbuffer);
	(*env)->DeleteLocalRef(env, cf_class);
}
int jfread(JNIEnv * env, jobject cf, void * buffer, int length)
{
	//java Array to hold the buffer contents
	jbyteArray jbuffer = (*env)->NewByteArray(env,length);
	
	//get class
	jclass cf_class = (*env)->GetObjectClass(env,cf);
	
	//call the actual function
 	jmethodID read = (*env)->GetMethodID(env, cf_class, "read", "([BII)I");
	int ret = (*env)->CallIntMethod(env,cf,read, jbuffer,0, length);
	
	(*env)->GetByteArrayRegion(env, jbuffer, 0,length, (byte *) buffer);
	
	(*env)->DeleteLocalRef(env, jbuffer);
	(*env)->DeleteLocalRef(env, cf_class);
	return ret;
}
void jfclose(JNIEnv * env, jobject cf)
{
	//get class
	jclass cf_class = (*env)->GetObjectClass(env,cf);
	jmethodID close = (*env)->GetMethodID(env, cf_class, "close", "()V");
	(*env)->CallVoidMethod(env,cf,close);
	
	(*env)->DeleteLocalRef(env, cf_class);
	(*env)->DeleteLocalRef(env, cf);
}