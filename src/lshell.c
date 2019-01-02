#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "lshell_readline.h"
#include "lshell_def.h"
#include "lshell.h"

extern command_s _my_cmd[COMMAND_MAX_NUM + 1]; /* 命令数组,下标从0开始 */
extern int _cmd_index;                         /* 当前第一个未使用的数组下标 */
extern char _promt[PROMT_MAX_LEN + 1];         /* 提示符 */
extern char *_input_buf;                       /* 输入 */
static int _input_cnt;                         /* 输入中除去命令后的参数个数,传递给回调函数*/
static char **_input_arg;                      /* 输入中除去命令后的每个参数,传递给回调函数*/
static char _err_msg[ERR_MSG_LEN + 1];         /* 错误信息*/
static char _err_msg_on;                       /* 错误信息开关*/

/* 参数：无*/
/* 说明：打印错误信息 */
/* 返回值：无 */
static void print_err_msg()
{
    if(_err_msg_on && strlen(_err_msg) != 0) 
    {
        printf("%s\n", _err_msg);
    }

    return;
}

/* 参数：待添加的命令*/
/* 说明：判断该命令是否存在 */
/* 返回值：0 不存在 */
/*       1  存在 */
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

/* 参数：命令*/
/* 返回值：该命令的id，若该命令不存在则返回-1 */
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

/* 参数：终端的输入*/
/* 说明：解析终端的输入 */
/* 返回值：该命令的id，若出错则返回-1 */
static int lshell_analysis_input(const char *input)
{
    int i = 0, j, id, len, input_cnt = 0;
    char input_arg[INPUT_ARG_CNT][ARGS_MAX_LEN + 1];

    len = strlen(input);
    if(len > INPUT_MAX_LEN)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Your input is to long!(1 ~ 2048)\n");
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
            snprintf(_err_msg, ERR_MSG_LEN, "There're too many args!(1 ~ 128)\n");
            return -1;
        }
        if(j - i > ARGS_MAX_LEN)
        {
            snprintf(_err_msg, ERR_MSG_LEN, "One of args is to long!(1 ~ 128)\n");
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

/* 参数：parent 父命令id，若无父命令，则为-1 */
/* 参数：cmd 命令 */
/* 参数：tip 命令的说明 */
/* 参数：func 函数指针 */
/* 说明：注册用户的命令 */
/* 返回值：该命令的id，若出错则返回-1 */
int lshell_register(int parent, const char *cmd, const char *tip, void (* func)(int argc, char **argv))
{
    int i;

    if(_cmd_index > COMMAND_MAX_NUM)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Command 's number is too more!(1 ~ 128)\n");
        return -1;
    }
    if(cmd_exist(cmd))
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Command: %s already existed!\n", cmd);
        return -1;
    }
    if(strlen(cmd) > COMMAND_MAX_LEN)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Command: %s's length is too long!(1 ~ 16)\n", cmd);
        return -1;
    }
    if(strlen(tip) > COMMAND_MAX_TIP)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Tip: %s's length is too long!(1 ~ 128)\n", tip);
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

/* 说明：设置命令提示符 */
void lshell_set_promt(const char *str)
{
    snprintf(_promt, PROMT_MAX_LEN, "%s>", str);

    return; 
}

/* 参数：0 关闭错误提示 */
/*      1 打开错误提示 */
/* 说明：设置错误提示开关 */
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

/* 说明：lshell初始化 */
void lshell_init()
{
    lshell_readline_init();
    lshell_set_promt("lshell");

    return;
}

/* 说明：lshell启动 */
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
