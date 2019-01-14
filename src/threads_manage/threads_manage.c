#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <pthread.h>

#include "threads_manage.h"

/***************************************************************/
/* 函  数：recycle_thread_res ***********************************/
/* 说  明：回收线程资源 *******************************************/
/* 参  数：th_header 线程管理头部 *********************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
static void recycle_thread_res(thread_header_s *th_header)
{
	int i;
	
	if(th_header->used == 0)
	{
		return;
	}
	
	for(i = 0; i < th_header->argc; i++)
	{
		free(th_header->argv[i]);
	}
	free(th_header->argv);
	memset(th_header, 0, sizeof(thread_header_s));
	
	return;
}

/***************************************************************/
/* 函  数：thread_func ******************************************/
/* 说  明：线程执行的函数 *****************************************/
/* 返回值：NULL *************************************************/
/**************************************************************/
static void *thread_func(void *arg)
{
	thread_header_s *th_header;
	
	pthread_detach(pthread_self()); 						  /* 设置线程的分离状态 */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);      /* 设置线程可被其他线程cansel */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); /* 设置线程收到cansel信号时立刻退出 */
	
	th_header = (thread_header_s *)arg;
	th_header->func(th_header->argc, th_header->argv);
	recycle_thread_res(th_header);
	
	return NULL;
}

/***************************************************************/
/* 函  数：thread_start *****************************************/
/* 说  明：线程启动函数 *******************************************/
/* 参  数：th_header 线程管理头部 *********************************/
/*        message 线程信息 **************************************/
/* 返回值：0 启动成功 ********************************************/
/*      ：1 启动失败 ********************************************/
/**************************************************************/
char thread_start(thread_header_s *th_header, char *message)
{
	struct timeval tv;
    
    gettimeofday(&tv, NULL);
	th_header->starttime_high = tv.tv_sec;
	th_header->starttime_low = tv.tv_usec;
	th_header->used = 1;
	strncpy(th_header->message, message, sizeof(th_header->message));
	
	if(pthread_create(&(th_header->tid), NULL, thread_func, (void *)th_header) != 0)
	{
		recycle_thread_res(th_header);
		printf("%s thread failed to start!\n", message);
		return 1;
	}
	
	return 0;
}

/***************************************************************/
/* 函  数：thread_stop ******************************************/
/* 说  明：线程中止函数 *******************************************/
/* 参  数：th_header 线程管理头部 *********************************/
/* 返回值：0 中止成功 ********************************************/
/*      ：1 中止失败 ********************************************/
/**************************************************************/
char thread_stop(thread_header_s *th_header)
{
	if(pthread_cancel(th_header->tid) != 0)
	{
		return 1;
	}
	recycle_thread_res(th_header);
	
	return 0;
}
