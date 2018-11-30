#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lshell.h"

#define COMMAND_MAX_NUM 127  /* 命令的最大数量 */ 
#define COMMAND_MAX_LEN 15   /* 每条命令及参数的最大长度 */
#define COMMAND_MAX_TIP 127  /* 命令说明的最大长度 */
#define PROMT_MAX_LEN   15   /* 提示符的最大长度 */
#define INPUT_MAX_LEN   2048 /* 输入的最大长度 */
#define INPUT_ARG_CNT   127  /* 输入中参数的最大个数 */
#define ARGS_MAX_LEN    127  /* 输入中参数的最大长度 */
#define ERR_MSG_LEN     127  /* 错误提示的最大长度 */

/* 结构化存储每条命令 */
typedef struct _command{
	int id;                        /* 每命令的id,通常为在结构体数组中的下标 */
	int parent;                    /* 该命令的父命令id */
	char cmd[COMMAND_MAX_LEN + 1]; /* 存储该命令 */
	char tip[COMMAND_MAX_TIP + 1]; /* 该命令的说明 */
	lshell_func func;              /* 该命令的函数指针 */
}command_s;

static command_s _my_cmd[COMMAND_MAX_NUM + 1]; /* 命令数组,下标从0开始 */
static int _cmd_index;                         /* 当前第一个未使用的数组下标 */
static char _promt[PROMT_MAX_LEN + 1];         /* 提示符 */
//static char _input_buf[INPUT_MAX_LEN * 2 + 1]; /* 每次的输入 */
char *_input_buf;
static int _input_cnt;                         /* 输入中除去命令后的参数个数,传递给回调函数*/
static char **_input_arg;                      /* 输入中除去命令后的每个参数,传递给回调函数*/
static char _err_msg[ERR_MSG_LEN];             /* 错误信息*/
static char _err_msg_on = 1;                   /* 错误信息开关*/

/* 参数：无*/
static void print_err_msg()
{
	if(_err_msg_on && strlen(_err_msg) != 0) 
	{
		printf("%s\n", _err_msg);
	}
	
	return;
}
static char cmd_exist(const char *str)
{
	int i = 0;
	
	while(i < _cmd_index)
	{
		if(strcmp(_my_cmd[i].cmd, str) == 0)
		{
			return 1;
		}
		i++;
	}
	
	return 0;
}

static int get_id_by_cmd(const char *cmd)
{
	int i;
	
	for(i = 0; i < _cmd_index; i++)
	{
		if(strcmp(_my_cmd[i].cmd, cmd) == 0)
		{
			return i;
		}
	}
	
	return -1;
}

static int lshell_analysis_input(const char *input)
{
	int i = 0, j, id, len, input_cnt = 0;
    char input_arg[INPUT_ARG_CNT][ARGS_MAX_LEN + 1];
    
    len = strlen(input);
    if(len > INPUT_MAX_LEN)
    {
		snprintf(_err_msg, ERR_MSG_LEN, "Your input is to long!(1 ~ 127)\n");
		return -1;
	}
	
	while(input[i] == ' ')
	{
		i++;
	}
	
	j = i;
	while(i < len)
	{
		while(input[j] != ' ' && j < len)
		{
			j++;
		}
		if(input_cnt > INPUT_ARG_CNT)
		{
			snprintf(_err_msg, ERR_MSG_LEN, "There're too many args!(1 ~ 31)\n");
			return -1;
		}
		if(j - i > ARGS_MAX_LEN)
		{
			snprintf(_err_msg, ERR_MSG_LEN, "One of args is to long!(1 ~ 15)\n");
			return -1;
		}
		if(j - i != 0)
		{
			snprintf(input_arg[input_cnt++], j - i + 1, "%s", (input + i));
		}
		if(j >= len)
		{
			break;
		}
		while(input[j] == ' ')
		{
			j++;
		}
		i = j;
	}
	
	id = get_id_by_cmd(input_arg[0]);
	if(id == -1)
	{
		return -1;
	}
	for(i = 1; i < input_cnt; i++)
	{
		j = get_id_by_cmd(input_arg[i]);
		if(j == -1)
		{
			break;
		}
		if(id == _my_cmd[j].parent)
		{
			id = j;
			_input_cnt--;
		}
		else
		{
			return -1;
		}
	}
	_input_cnt = input_cnt - 1;
	_input_arg = (char **)malloc(_input_cnt * sizeof(char *));
	for(i = input_cnt - _input_cnt, j = 0; i < input_cnt; i++)
	{
		_input_arg[j] =  (char *)malloc((strlen(input_arg[i]) + 1) * sizeof(char));
		sprintf(_input_arg[j], "%s", input_arg[i]);
		j++;
	}
	
	return id;	
}

int lshell_register(int parent, const char *cmd, const char *tip, lshell_func func)
{
	int i;
	
	if(_cmd_index > COMMAND_MAX_NUM)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Command 's number is too more!(1 ~ 127)\n");
		return -1;
	}
	if(cmd_exist(cmd))
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Command: %s already existed!\n", cmd);
		return -1;
	}
	if(strlen(cmd) > COMMAND_MAX_LEN)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Command: %s's length is too long!(1 ~ 15)\n", cmd);
		return -1;
	}
	if(strlen(tip) > COMMAND_MAX_TIP)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Tip: %s's length is too long!(1 ~ 127)\n", tip);
		return -1;
	}
	
	i = _cmd_index++;
	_my_cmd[i].id = i;
	_my_cmd[i].parent = parent;
	strcpy(_my_cmd[i].cmd, cmd);
	strcpy(_my_cmd[i].tip, tip);
	_my_cmd[i].func = func;
	
	return i;
}

void lshell_set_promt(const char *str)
{
	snprintf(_promt, PROMT_MAX_LEN, "%s>", str);
	
	return; 
}

void lshell_init()
{
	lshell_set_promt("lshell");
	
	return;
}

void lshell_start()
{
	int ret, i;
	
	while(1)
	{
		memset(_err_msg, 0, sizeof(_err_msg));
		
		free(_input_buf);
		_input_buf = readline(_promt);
		if(feof(stdin)) 
		{
			printf("\n");
			return;
		}
		if(strlen(_input_buf) == 0)
		{
			continue;
		}
		ret = lshell_analysis_input(_input_buf);
		print_err_msg();
		
		if(ret == -1)
		{
			printf("Invailed input!\n");
			
		}
		else
		{
			_my_cmd[ret].func(_input_cnt, _input_arg);
			for(i = 0; i < _input_cnt; i++)
			{
				free(_input_arg[i]);
			}
			free(_input_arg);
			_input_arg = NULL;
		}
	}
		
	return;		
}
