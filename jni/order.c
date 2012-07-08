#include "inc/order.h"
/*
	results for non-blocking give* Orders:

		- -7: successfully queued


	for blocking (can set result after processing order):

		- -7 standard, unchanged, set while enqueueing
*/
int process_orders(void)
{
	order_t * cur;

	//warining: if the order ressource (ie cur->val) ist still needed after execution of process_orders or the giving order method in the blocking case then new memory needs to be allocated for it

	if(order_queue->count)
	{
		while(cur = (order_t*) queue_dequeue(order_queue))
		{
			//is the order blocking?
			if(cur->mutex)
			{
				//yes
				pthread_mutex_lock(cur->mutex);
			}

			switch(cur->request)
			{
				//string orders
				case O_LOAD_ROM:
					cur->res = rom_load((char *) cur->val);
				break;

				case O_LOAD_STATE:
					cur->res = state_load((char *) cur->val);
				break;

				case O_SAVE_STATE:
					cur->res = state_save((char *) cur->val);
				break;

				case O_SCREENSHOT:
					order_take_screenshot((char *) cur->val);
				break;

				case O_SAVE_CONFIG:
					if(rom_get_loaded())
					{
						save_config((char *) cur->val);
					}
				break;

				case O_SAVE_CONFIG_GLOBAL:
					if(rom_get_loaded())
					{
						save_config_glob((char *) cur->val);
					}
				break;

				case O_LOAD_CONFIG:
					load_config((char *) cur->val);

				break;

				case O_LOAD_CONFIG_GLOBAL:
					load_config_glob((char *) cur->val);
				break;

				//int orders
				case O_REWIND:
					if(*(int *)cur->val)
						rin_rewind = 1;
					else
						rin_rewind = 0;

					cur->res = 1;
				break;
				
			}

			if(cur->mutex)
			{
				//blocking
				//let the waiting thread know that the order has been processed
				pthread_cond_signal(cur->cond_var);
				pthread_mutex_unlock(cur->mutex);

				//the order-giving thread will now free all components of the order (value, mutex, conditino and the order itself)
			}
			else
			{
				//non blocking order
				//its on us to free the allocated order and its value
				free(cur->val);
				free(cur);
			}
		}
	}
	return 1;
}
JNIEXPORT int JNICALL Java_org_rin_Order_giveString(JNIEnv *envs, jobject this, jint request, jstring val, jint length)
{
	char * tmp = malloc(sizeof(char) * (length+1));
	char * ptmp = (char *) (*envs)->GetStringUTFChars(envs, val, 0);
	strcpy(tmp,ptmp);
	(*envs)->ReleaseStringUTFChars(envs, val, ptmp);

	order_t * tmporder = malloc(sizeof(order_t));
	tmporder->request = (int)request;
	tmporder->val = (void *) tmp;

	tmporder->res 		 = -100;
	tmporder->mutex		 = NULL;
	tmporder->cond_var	 = NULL;

	queue_enqueue(order_queue,tmporder);
	return -7;
}
JNIEXPORT int JNICALL Java_org_rin_Order_giveInt(JNIEnv *envs, jobject this, jint request, jint val)
{
	order_t * tmporder = malloc(sizeof(order_t));
	tmporder->request = (int)request;
	tmporder->val = malloc(sizeof(int));
	*(int*)tmporder->val = (int) val;

	tmporder->res 		 = -100;
	tmporder->mutex		 = NULL;
	tmporder->cond_var	 = NULL;
	queue_enqueue(order_queue,tmporder);
	return -7;
}
JNIEXPORT int JNICALL Java_org_rin_Order_giveStringBlocking(JNIEnv *envs, jobject this, jint request, jstring val, jint length)
{
	int result;
	char * ptmp = (char *) (*envs)->GetStringUTFChars(envs, val, 0);
	char * tmp = malloc(sizeof(char) * (length+1));
	strcpy(tmp,ptmp);
	(*envs)->ReleaseStringUTFChars(envs, val, ptmp);

	order_t * tmporder = malloc(sizeof(order_t));
	tmporder->request = (int)request;
	tmporder->val = (void *) tmp;
	tmporder->res 		 = -7;

	tmporder->mutex = malloc(sizeof(pthread_mutex_t));
	tmporder->cond_var = malloc(sizeof(pthread_cond_t));	
	pthread_mutex_init(tmporder->mutex,NULL);
	pthread_cond_init(tmporder->cond_var,NULL);


	pthread_mutex_lock(tmporder->mutex);
	queue_enqueue(order_queue,tmporder);
	pthread_cond_wait(tmporder->cond_var, tmporder->mutex);
	result = tmporder->res;

	free(tmporder->mutex);
	free(tmporder->cond_var);
	free(tmporder->val);
	free(tmporder);
	
	return result;
}
JNIEXPORT int JNICALL Java_org_rin_Order_giveIntBlocking(JNIEnv *envs, jobject this, jint request, jint val)
{
	int result;
	order_t * tmporder = malloc(sizeof(order_t));
	tmporder->request = (int)request;
	tmporder->res 		 =-7;
	tmporder->val = malloc(sizeof(int));
	*(int*)tmporder->val = (int) val;


	tmporder->mutex = malloc(sizeof(pthread_mutex_t));
	tmporder->cond_var = malloc(sizeof(pthread_cond_t));	
	pthread_mutex_init(tmporder->mutex,NULL);
	pthread_cond_init(tmporder->cond_var,NULL);

	pthread_mutex_lock(tmporder->mutex);
	queue_enqueue(order_queue,tmporder);
	pthread_cond_wait(tmporder->cond_var, tmporder->mutex);
	result = tmporder->res;

	free(tmporder->mutex);
	free(tmporder->cond_var);
	free(tmporder->val);
	free(tmporder);

	return result;
}
void order_take_screenshot(char * filename)
{
	unsigned short *vptr = (unsigned short*)pgGetVramAddr(0, 0);
	saveImage(filename, vptr, dscreen_width, dscreen_height, dline_size, 0);
}

