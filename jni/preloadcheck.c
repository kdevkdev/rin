#include <cpu-features.h>
#include <jni.h>


//use this to detect wether java should load a library that uses neon 
//can be generalized in the feature to support more runtime detection possibilites (for examle SSE3 on x86), but for now keep it simple


JNIEXPORT jboolean JNICALL Java_org_rin_RinService_detectNeon(JNIEnv *envs, jobject this, jstring rpath)
{
	uint64_t features = android_getCpuFeatures();
	
	if((android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM) && (ANDROID_CPU_ARM_FEATURE_NEON & features))
	{
		return JNI_TRUE;
	}
	else
	{
		return JNI_FALSE;
	}
}
