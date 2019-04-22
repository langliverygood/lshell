#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>

#include "threads_manage.h"
#include "lshell_def.h"

command_s _my_cmd[COMMAND_MAX_NUM];        /* 命令数组,下标从0开始 */
int _cmd_index;                            /* 当前第一个未使用的数组下标 */
char _promt[PROMT_MAX_LEN + 1];            /* 提示符 */
int _input_cnt;                            /* 输入中除去命令后的参数个数,传递给回调函数 */
char **_input_arg;                         /* 输入中除去命令后的每个参数,传递给回调函数 */
char _err_msg[ERR_MSG_LEN + 1];            /* 错误信息 */
char _err_msg_on;                          /* 错误信息开关 */
thread_header_s th_header[THREAD_MAX_NUM]; /* 线程管理结构体数组 */

/***************************************************************/
/* 说  明：获得线程运行时间 ****************************************/
/***************************************************************/
static char * get_run_time(struct timeval start, struct timeval end, char *out_str)
{
	int days, hours, minites, seconds, m_seconds;
	char m_seconds_str[4];
	
	if(start.tv_usec > end.tv_usec)
	{
		end.tv_sec--;
		end.tv_usec += 1000000;
	}
	m_seconds = end.tv_usec - start.tv_usec;
	seconds = end.tv_sec - start.tv_sec;
	
	days = 0;
	hours = 0;
	minites = 0;
	while(seconds > 86400)
	{
		days++;
		seconds -= 86400;
	}
	while(seconds > 3600)
	{
		hours++;
		seconds -= 3600;
	}
	while(seconds > 60)
	{
		minites++;
		seconds -= 60;
	}
	snprintf(m_seconds_str, sizeof(m_seconds_str), "%d", m_seconds);
	sprintf(out_str, "%dd %dh %dm %d.%ss", days, hours, minites, seconds, m_seconds_str);
	
	return out_str;
}
 
/***************************************************************/
/* 说  明：预定义help命令 *****************************************/
/***************************************************************/
void lshell_help(int argc, char **argv)
{
	int i;
	char tmp[COMMAND_MAX_NUM];
	
	if(argc == 0)
	{
		for(i = 0; i < _cmd_index; i++)
		{
			printf("%*s\"%s\"\n", -64, _my_cmd[i].cmd, _my_cmd[i].tip);
		}
		return;
	}
	
	memset(tmp, 0, sizeof(tmp));
	for(i = 0; i < argc; i++)
	{
		strcat(tmp, argv[i]);
		strcat(tmp, " ");
	}
	tmp[strlen(tmp) - 1] = '\0';
	
	for(i = 0; i < _cmd_index; i++)
	{
		if(strcmp(tmp, _my_cmd[i].cmd) == 0)
		{
			printf("\"%s\"\n",_my_cmd[i].tip);
			return;
		}
	}
	printf("Cmd :%s does not exist!\n", tmp);
	
	return;
}

/***************************************************************/
/* 说  明：预定义exit命令 *****************************************/
/***************************************************************/
void lshell_exit(int argc, char **argv)
{
    printf("Bye!\n");
    
	exit(0);
}

/***************************************************************/
/* 说  明：预定义threads命令 **************************************/
/***************************************************************/
void lshell_threads(int argc, char **argv)
{
	int i;
	char time[128];
	struct timeval tv;
	struct timeval tv_tmp;
	
	gettimeofday(&tv, NULL);
	printf("id%*stid%*srun_time%*smessage\n", 3, "", 21, "", 23, "");
	for(i = 0; i < THREAD_MAX_NUM; i++)
	{
		if(th_header[i].used == 1)
		{
			tv_tmp.tv_sec = th_header[i].starttime_high;
			tv_tmp.tv_usec = th_header[i].starttime_low;
			memset(time, 0, sizeof(time));
			get_run_time(tv_tmp, tv, time);
			printf("%-4d %-22lu  %-30.30s \"%s\"\n", i, th_header[i].tid, time, th_header[i].message);
		}
	}
    
	return;
}

/***************************************************************/
/* 说  明：预定义kill令 ******************************************/
/***************************************************************/
void lshell_kill(int argc, char **argv)
{
	int i, id;
	long val;
	char *endptr, c;
	
	if(argc == 0)
	{
		printf("Do you want to kill all child threads?[Y/N]");
		scanf(" %c", &c);
		if(c == 'y' || c == 'Y')
		{
			for(i = 0; i < THREAD_MAX_NUM; i++)
			{
				if(th_header[i].used == 1)
				{
					thread_stop(&th_header[i]);
				}
			}
		}
	}
	else
	{
		for(i = 0; i < argc; i++)
		{
			errno = 0;
			val = strtol(argv[i], &endptr, 0);
			if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))|| (errno != 0 && val == 0)) 
			{
				continue;
			}
			if(endptr == argv[i]) 
			{
			   continue;
			}
			id = (int)val;
			if(id >= 0 && id < THREAD_MAX_NUM)
			{
				thread_stop(&th_header[id]);
			}
		}
	}
	
	return;
}

/***************************************************************/
/* 函  数：get_first_unused_th_header() *************************/
/* 说  明：返回当前第一个未使用的线程头结构体的下标 ********************/
/* 参  数：无 ****************************************************/
/* 返回值：下标 ***************************************************/
/*      ：-1 结构体全被占用 ***************************************/
/***************************************************************/
static int get_first_unused_th_header()
{
	int i;
	
	for(i = 0; i < THREAD_MAX_NUM; i++)
	{
		if(th_header[i].used == 0)
		{
			return i;
		}
	}
	
	return -1;
}

/***************************************************************/
/* 函  数：start_new_thread() ***********************************/
/* 说  明：启动新线程去执行 ****************************************/
/* 参  数：_my_cmd_index 命令结构体的下标 **************************/
/*        message 线程说明信息 ***********************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
void start_new_thread(int _my_cmd_index, char *message)
{
	int i, index;

	index = get_first_unused_th_header();
	if(index == -1)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "The number of threads the program can handle has reached its maximum\n");
		return;
	}
	
	/* 添加线程管理头部信息 */
	th_header[index].argc = _input_cnt;
	for(i = 0; i < _input_cnt; i++)
	{
		strcpy(th_header[index].argv_storage[i], _input_arg[i]);
		th_header[index].argv[i] = th_header[index].argv_storage[i];
	}
	th_header[index].func = _my_cmd[_my_cmd_index].func;
	th_header[index].join_detach = _my_cmd[_my_cmd_index].join_detach;
	th_header[index].cancelstate = _my_cmd[_my_cmd_index].cancelstate;
	th_header[index].canceltype = _my_cmd[_my_cmd_index].canceltype;
	th_header[index].used = 1;
	
	/* 启动新线程 */
	thread_start(&th_header[index], message);
	
	return;
}
