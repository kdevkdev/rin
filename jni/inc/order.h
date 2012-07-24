#ifndef ORDER_H
#define ORDER_H

#include "main.h"

//VALID ORDERS -- highest order upt to now: 9 !MUST! be identical with the numbers in Order.java
//*********************************
//of type string
#define O_LOAD_ROM		1
#define O_LOAD_STATE		2
#define O_SAVE_STATE		3
#define O_SCREENSHOT		5
#define O_LOAD_CONFIG		6
#define O_SAVE_CONFIG		7
#define O_LOAD_CONFIG_GLOBAL 	8
#define O_SAVE_CONFIG_GLOBAL	9

//of type int
#define O_REWIND 	4

//**********************************


struct orderstruct{
	int request;
	void * val;
	int res;

	//for blocking messages
	pthread_mutex_t  * mutex;
	pthread_cond_t * cond_var;
};
typedef struct orderstruct order_t;

//called from the mainloop
int process_orders(void);

//should only be called from java
JNIEXPORT int JNICALL Java_org_rin_Order_giveString(JNIEnv *envs, jobject this, jint request, jstring val, jint length);
JNIEXPORT int JNICALL Java_org_rin_Order_giveInt(JNIEnv *envs, jobject this, jint request, jint val);



//internal use
//void order_take_screenshot(char * filename);

#endif

