#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>

#include "threads_manage.h"
#include "lshell_def.h"

extern command_s _my_cmd[COMMAND_MAX_NUM];                                     /* 命令数组,下标从0开始 */
extern int _cmd_index;                                                         /* 当前第一个未使用的数组下标 */
extern char _cmd_all[COMMAND_MAX_NUM][COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1)];/* 父命令和子命令拼接后的所有命令 */
extern int _input_cnt;                                                         /* 输入中除去命令后的参数个数,传递给回调函数 */
extern char **_input_arg;                                                      /* 输入中除去命令后的每个参数,传递给回调函数 */
thread_header_s th_header[THREAD_MAX_NUM];                                     /* 线程管理结构体数组 */

/***************************************************************/
/* 说  明：预定义help命令 *****************************************/
/***************************************************************/
void lshell_help(int argc, char **argv)
{
	int i;
	char cmd_tmp[COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1)];
	
	if(argc == 0)
	{
		for(i = 0; i < _cmd_index; i++)
		{
			printf("%*s\"%s\"\n", -COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1), _cmd_all[i], _my_cmd[i].tip);
		}
		return;
	}
	
	memset(cmd_tmp, 0, sizeof(cmd_tmp));
	
	for(i = 0; i < argc; i++)
	{
		strcat(cmd_tmp, argv[i]);
		strcat(cmd_tmp, " ");
	}
	cmd_tmp[strlen(cmd_tmp) - 1] = '\0';
	for(i = 0; i < _cmd_index; i++)
	{
		if(strcmp(cmd_tmp, _cmd_all[i]) == 0)
		{
			printf("%*s\"%s\"\n", -COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1), _cmd_all[i], _my_cmd[i].tip);
			return;
		}
	}
	printf("Cmd :%s does not exist!\n", cmd_tmp);
	
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
	
	printf("id%*stid%*smessage\n", 2, "", 20, "");
	for(i = 0; i < THREAD_MAX_NUM; i++)
	{
		if(th_header[i].used == 1)
		{
			printf("%*s\"%s\"\n", -COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1), _cmd_all[i], _my_cmd[i].tip);
			printf("%-4d%-23lu%*s\"%s\"\n", i, th_header[i].tid, 5, "", th_header[i].message);
		}
	}
    
	return;
}

/***************************************************************/
/* 说  明：预定义kill令 ******************************************/
/***************************************************************/
void lshell_kill(int argc, char **argv)
{
	int i, j;
	long val;
	char *endptr, c;
	pthread_t tid;
	
	if(argc == 0)
	{
		printf("Do you want to kill all child threads?[Y/N]");
		scanf(" %c", &c);
		if(c == 'y' || c == 'Y')
		{
			for(j = 0; j < THREAD_MAX_NUM; j++)
			{
				if(th_header[j].used == 1)
				{
					thread_stop(&th_header[j]);
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
			tid = (pthread_t)val;
			for(j = 0; j < THREAD_MAX_NUM; j++)
			{
				if(th_header[j].used == 1 && th_header[j].tid == tid)
				{
					thread_stop(&th_header[j]);
					break;
				}
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
/* 参  数：命令结构体的下标 ****************************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
void start_new_thread(int _my_cmd_index)
{
	int i;
	
	i = get_first_unused_th_header();
	if(i == -1)
	{
		printf("The number of threads the program can handle has reached its maximum\n");
		return;
	}
	th_header[i].argc = _input_cnt;
	th_header[i].argv = _input_arg;
	th_header[i].func = _my_cmd[_my_cmd_index].func;
	thread_start(&th_header[i], _my_cmd[_my_cmd_index].tip);
	
	return;
}
