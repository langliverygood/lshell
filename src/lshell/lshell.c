#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "lshell_readline.h"
#include "lshell_def.h"
#include "lshell.h"

extern command_s _my_cmd[COMMAND_MAX_NUM];       /* 命令数组,下标从0开始 */
extern int _cmd_index;                           /* 当前第一个未使用的数组下标 */
extern char _promt[PROMT_MAX_LEN + 1];           /* 提示符 */
extern int _input_cnt;                           /* 输入中除去命令后的参数个数,传递给回调函数 */
extern char **_input_arg;                        /* 输入中除去命令后的每个参数,传递给回调函数 */
extern char _err_msg[ERR_MSG_LEN + 1];           /* 错误信息 */
extern char _err_msg_on;                         /* 错误信息开关 */

/***************************************************************/
/* 函  数：print_err_msg() **************************************/
/* 说  明：当错误信息开关打开时，打印错误信息 *************************/
/* 参  数：无 ****************************************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
static void print_err_msg()
{
    if(_err_msg_on && strlen(_err_msg) != 0) 
    {
        printf("%s\n", _err_msg);
    }

    return;
}

/***************************************************************/
/* 函  数：get_cmd_index ****************************************/
/* 说  明：获得命令在结构体中的下标 *********************************/
/* 参  数：cmd 待判断的命令 ***************************************/
/* 返回值：存在 该命令结构体的下标 **********************************/
/*      ：不存在 -1 *********************************************/
/**************************************************************/
static int get_cmd_index(char *cmd)
{
    int i;

    for(i = 0; i < _cmd_index; i++)
    {
        if(strcmp(cmd, _my_cmd[i].cmd) == 0)
        {
            return i;
        }
    }

    return -1;
}

/***************************************************************/
/* 函  数：lshell_analysis_input ********************************/
/* 说  明：解析终端输入 *******************************************/
/* 参  数：input 终端输入 ****************************************/
/* 返回值：成功 命令在结构体数组中的下标 *****************************/
/*      ：失败 -1 ***********************************************/
/***************************************************************/
static int lshell_analysis_input(const char *input)
{
    int i, j, id, len, argc;
    char args[COMMAND_MAX_LEN][COMMAND_MAX_LEN + 1];
    char tmp[COMMAND_MAX_LEN + 1];

    len = strlen(input);
    if(len > INPUT_MAX_LEN)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Your input is to long!(1 ~ 2047)\n");
        return -1;
    }
    
    /* 清空输入首部的空格 */
    i = 0;
    while(input[i] == ' ')
    {
        i++;
    }
    /* 根据空格将输入划分为字符串数组,存储在局部变量input_arg中 */
    j = i;
    argc = 0; 
    while(i < len)
    {
        while(input[j] != ' ' && j < len)
        {
            j++;
        }
        if(j - i != 0)
        {
            snprintf(args[argc++], j - i + 1, "%s", (input + i));
        }
        if(j >= len)
        {
            break;
        }
        /* 清空输入中间的空格*/
        while(input[j] == ' ')
        {
            j++;
        }
        i = j;
    }
    
    /* 在字符串数组中，最多前i个可以组成命令 */
    i = 1;
    sprintf(tmp, "%s", args[0]);
    id = j = get_cmd_index(tmp);
    while(j != -1)
    {
		id = j;
		strcat(tmp, " ");
		strcat(tmp, args[i]);
		j = get_cmd_index(tmp);
		i++;
	}
	i--;
	
    /* input_arg去除命令后,剩下的元素即为参数,存储到全局变量_input_arg中, 参数个数存储到全局变量_input_cnt */
    _input_cnt = argc - i;
    _input_arg = (char **)malloc(_input_cnt * sizeof(char *));
    for(j = 0; i < argc; i++, j++)
    {
        _input_arg[j] =  (char *)malloc((strlen(args[i]) + 1) * sizeof(char));
        sprintf(_input_arg[j], "%s", args[i]);
    }

    return id;
}

/********************************************************************************************/
/* 函  数：lshell_register *******************************************************************/
/* 说  明：注册命令 ***************************************************************************/
/* 参  数：parent 父命令id，若无父命令，则为-1 ***************************************************/
/*      ：cmd 待注册的命令 ********************************************************************/
/*      ：tip 待注册命令的提示 *****************************************************************/
/*      ：func 函数指针 ***********************************************************************/
/*      ：mode 命令执行方式，在主线程中0/在主线程中非0 *********************************************/
/*      ：join_detach 当mode=0时有效,0：joined 1：detached  ************************************/
/*      ：cancelstate 当mode=0时有效, 0表示收到信号后设为CANCLED状态,非0忽略CANCEL信号继续运行********/
/*      ：canceltype 当mode=0时有效, 表示收到信号后继续运行至下一个取消点再退出,1立即执行取消动作(退出)***/
/* 返回值：成功 该命令在结构体数组中的下标 *********************************************************/
/*      ：失败 -1 *****************************************************************************/
/*********************************************************************************************/
int lshell_register(int parent, const char *cmd, const char *tip, void (* func)(int argc, char **argv), 
                                                  int mode, int join_detach, int cancelstate, int canceltype)
{
    int i, index;
    char args[COMMAND_MAX_LEN][COMMAND_MAX_LEN + 1];
    char tmp[COMMAND_MAX_LEN + 1];
    
    /* 检查参数合法性 */
    if(parent < -1)
    {
		snprintf(_err_msg, ERR_MSG_LEN, "Parent value error!\n");
		return -1;
	}
	if(strlen(cmd) > COMMAND_MAX_LEN)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Cmd is too long!(1 ~ %d)\n", COMMAND_MAX_LEN);
		return -1;
	}
	if(strlen(tip) > COMMAND_MAX_TIP)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Tip is too long!(1 ~ %d)\n", COMMAND_MAX_TIP);
		return -1;
	}
    
    /* 根据父子关系拼接出完整的命令 */
    memset(args, 0, sizeof(args));
    sprintf(args[0], "%s", cmd);
    for(i = 1, index = parent; index != -1; i++)
    {
		if(i == COMMAND_MAX_LEN)
		{
			break;
		}
		sprintf(args[i], "%s", _my_cmd[index].cmd);
		index = _my_cmd[index].parent;
	}
	sprintf(tmp, "%s", args[--i]);
	for(--i; i>= 0; --i)
	{
		strcat(tmp, " ");
		strcat(tmp, args[i]);
	}
	
	/* 如果命令不存在则可以注册 */
	if(get_cmd_index(tmp) != -1)
	{
		snprintf(_err_msg, ERR_MSG_LEN, "Command: %s already existed!\n", tmp);
		return -1;
	}
	_my_cmd[_cmd_index].id = _cmd_index;
    _my_cmd[_cmd_index].parent = parent;
    strcpy(_my_cmd[_cmd_index].cmd, tmp);
    strcpy(_my_cmd[_cmd_index].tip, tip);
    _my_cmd[_cmd_index].func = func;
    _my_cmd[_cmd_index].mode = mode;
    if(mode == RUN_AT_NEW_THREAD)
    {
		_my_cmd[_cmd_index].join_detach = join_detach;
		_my_cmd[_cmd_index].cancelstate = cancelstate;
		_my_cmd[_cmd_index].canceltype = canceltype;
	}
	_cmd_index++;
	
	return (_cmd_index - 1);
}

/***************************************************************/
/* 函  数：lshell_set_promt *************************************/
/* 说  明：注册命令 **********************************************/
/* 参  数：pmt 提示符 ********************************************/
/* 返回值：无 ***************************************************/
/***************************************************************/
void lshell_set_promt(const char *pmt)
{
    snprintf(_promt, PROMT_MAX_LEN, "%s>", pmt);

    return; 
}

/***************************************************************/
/* 函  数：lshell_set_errmsg_swtich *****************************/
/* 说  明：设置错误提示开关 ****************************************/
/* 参  数：flag 0  关闭错误提示 ***********************************/
/*             1  打开错误提示 ***********************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
void lshell_set_errmsg_swtich(int flag)
{
    if(flag)
    {
        _err_msg_on = 1;
    }
    else
    {
        _err_msg_on = 0;
    }
    
    return;
}

/***************************************************************/
/* 函  数：lshell_init *******************************************/
/* 说  明：lshell初始化 ******************************************/
/* 参  数：无 ***************************************************/
/* 返回值：无 ***************************************************/
/***************************************************************/
void lshell_init()
{
    lshell_readline_init();
    lshell_set_promt("lshell");
    lshell_register(-1, "exit", "Close the program.", lshell_exit, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_register(-1, "help", "Show the commands' help.", lshell_help, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_register(-1, "threads", "Show all the child threads", lshell_threads, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_register(-1, "kill", "Kill the thread(s)", lshell_kill, RUN_AT_MAIN_THREAD, 0, 0, 0);

    return;
}

/****************************************************************/
/* 函  数：lshell_start ******************************************/
/* 说  明：lshell启动 ********************************************/
/* 参  数：无 ****************************************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
void lshell_start()
{
    int ret, i;
    char *mline;

    while(1)
    {
        mline = lshell_gets();
        memset(_err_msg, 0, sizeof(_err_msg));

        if(strlen(mline) == 0)
        {
            continue;
        }
        ret = lshell_analysis_input(mline);
        
        if(ret == -1)
        {
            printf("Invailed input!\n");

        }
        else
        {
			if(_my_cmd[ret].mode == RUN_AT_NEW_THREAD)
			{
				start_new_thread(ret, _my_cmd[ret].tip);
			}
			else
			{
				_my_cmd[ret].func(_input_cnt, _input_arg);
			}
        }
        for(i = 0; i < _input_cnt; i++)
		{
			free(_input_arg[i]);
		}
		free(_input_arg);
		_input_arg = NULL;
        print_err_msg();
    }

    return;
}
