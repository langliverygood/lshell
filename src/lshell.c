#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "lshell_readline.h"
#include "lshell_def.h"
#include "lshell.h"

extern command_s _my_cmd[COMMAND_MAX_NUM];                                     /* 命令数组,下标从0开始 */
extern int _cmd_index;                                                         /* 当前第一个未使用的数组下标 */
extern char _promt[PROMT_MAX_LEN + 1];                                         /* 提示符 */
extern char _cmd_all[COMMAND_MAX_NUM][COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1)];/* 父命令和子命令拼接后的所有命令 */
static int _input_cnt;                                                         /* 输入中除去命令后的参数个数,传递给回调函数 */
static char **_input_arg;                                                      /* 输入中除去命令后的每个参数,传递给回调函数 */
static char _err_msg[ERR_MSG_LEN + 1];                                         /* 错误信息 */
static char _err_msg_on;                                                       /* 错误信息开关 */

/***************************************************************/
/**函  数：static void print_err_msg() **************************/
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
/**函  数：static int get_cmd_index(char *cmd) ******************/
/* 说  明：在_cmd_all中判断命令是否存在 ****************************/
/* 参  数：cmd 待判断的命令 ***************************************/
/* 返回值：存在 该命令在_cmd_all 中的下标 **************************/
/*      ：不存在 -1 *********************************************/
/**************************************************************/
static int get_cmd_index(char *cmd)
{
    int i;

    for(i = 0; i < _cmd_index; i++)
    {
        if(strcmp(cmd, _cmd_all[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

/***************************************************************/
/**函  数：static int cmd_exist(char **arg, int num) ************/
/* 说  明：在_cmd_all中判断命令是否存在 ****************************/
/* 参  数：arg 终端输入根据空格划分后的字符串数组 ********************/
/*      ：num 字符串数组前num个元素需要拼接后进行判断 ****************/
/* 返回值：存在 该命令在_cmd_all 中的下标 **************************/
/*      ：不存在 -1 *********************************************/
/**************************************************************/
static int cmd_exist(char *arg, int num)
{
    int i;
    char tmp[COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1)];
    
    if(num == 0)
    {
        return -1;
    }
    
    /* 字符串数组中前num个元素进行拼接 */
    memset(tmp, 0, sizeof(tmp));
    for(i = 0; i < num; i++)
    {
        strcat(tmp, &arg[i * (ARGS_MAX_LEN + 1)]);
        strcat(tmp, " ");
    }
    tmp[strlen(tmp) - 1] = '\0';
    
    /* 判断拼接后的命令是否存在,若存在,同时把num的值赋给全局变量_input_cnt */
    for(i = 0; i < _cmd_index; i++)
    {
        if(strcmp(tmp, _cmd_all[i]) == 0)
        {
            _input_cnt = num;
            return i;
        }
    }

    return cmd_exist(arg, num - 1);
}

/***************************************************************/
/**函  数：static int get_id_by_cmd(const char *cmd) ************/
/* 说  明：获得命令在结构体数组中的下标 ******************************/
/* 参  数：cmd 待判断的命令 ***************************************/
/* 返回值：存在 该命令在结构体数组中的下标 ***************************/
/*      ：不存在 -1 *********************************************/
/***************************************************************/
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

/***************************************************************/
/**函  数：static int lshell_analysis_input(const char *input) **/
/* 说  明：解析终端输入 *******************************************/
/* 参  数：input 终端输入 ****************************************/
/* 返回值：成功 最终子命令在结构体数组中的下标 ************************/
/*      ：失败 -1 ***********************************************/
/***************************************************************/
static int lshell_analysis_input(const char *input)
{
    int i, j, id, len, input_cnt;
    char input_arg[INPUT_ARG_CNT][ARGS_MAX_LEN + 1];

    len = strlen(input);
    if(len > INPUT_MAX_LEN)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Your input is to long!(1 ~ 2047)\n");
        return -1;
    }

    i = 0;
    while(input[i] == ' ')
    {
        i++;
    }

    j = i;
    input_cnt = 0;
    /* 根据空格将输入划分为字符串数组,存储在局部变量input_arg中 */
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
            snprintf(_err_msg, ERR_MSG_LEN, "One of args is to long!(1 ~ 127)\n");
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
    
    /* 找到第一个在结构体数组中不存在的参数 */
    for(i = 0; i < input_cnt; i++)
    {
        if(get_id_by_cmd(input_arg[i]) == -1)
        {
            break;
        }
    }
    /* 判断命令是否存在 */
    id = cmd_exist((char *)input_arg, i);
    if(id == -1)
    {
        return -1;
    }
    
    id = get_id_by_cmd(input_arg[_input_cnt - 1]);
    /* input_arg去除命令后,剩下的元素即为参数,存储到全局变量_input_arg中, 参数个数存储到全局变量_input_cnt */
    _input_cnt = input_cnt - _input_cnt;
    _input_arg = (char **)malloc(_input_cnt * sizeof(char *));
    for(j = 0; i < input_cnt; i++)
    {
        _input_arg[j] =  (char *)malloc((strlen(input_arg[i]) + 1) * sizeof(char));
        sprintf(_input_arg[j], "%s", input_arg[i]);
        j++;
    }

    return id;
}

/***************************************************************/
/**函  数：int lshell_register(int parent...) *******************/
/* 说  明：注册命令 **********************************************/
/* 参  数：parent 父命令id，若无父命令，则为-1 **********************/
/* 参  数：cmd 待注册的命令 ***************************************/
/* 参  数：tip 待注册命令的提示 ***********************************/
/* 参  数：func 函数指针 *****************************************/
/* 返回值：成功 该命令在结构体数组中的下标 ***************************/
/*      ：失败 -1 ***********************************************/
/***************************************************************/
int lshell_register(int parent, const char *cmd, const char *tip, void (* func)(int argc, char **argv))
{
    int i;
    char tmp[COMMAND_MAX_DEP][COMMAND_MAX_DEP * (COMMAND_MAX_LEN + 1)];

    if(_cmd_index > COMMAND_MAX_NUM)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Command 's number is too more!(1 ~ 128)\n");
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

    _my_cmd[_cmd_index].id = _cmd_index;
    _my_cmd[_cmd_index].parent = parent;
    strcpy(_my_cmd[_cmd_index].cmd, cmd);
    strcpy(_my_cmd[_cmd_index].tip, tip);
    _my_cmd[_cmd_index].func = func;
    
    i = 0;
    memset(tmp, 0, sizeof(tmp));
    while(parent != -1)
    {
        strcpy(tmp[i++], _my_cmd[parent].cmd);
        parent = _my_cmd[parent].parent;
        if(i > COMMAND_MAX_DEP - 1)
        {
            snprintf(_err_msg, ERR_MSG_LEN, "Their hierarchy is too complex!(%s) \n", cmd);
            return -1;
        }
    }
    for(i--; i >= 0; i--)
    {
        strcat(_cmd_all[_cmd_index], tmp[i]);
        strcat(_cmd_all[_cmd_index], " ");
    }
    strcat(_cmd_all[_cmd_index], cmd);
    
    /* 若命令已经存在，则清空当前的赋值 */
    if(get_cmd_index(_cmd_all[_cmd_index]) != -1)
    {
        snprintf(_err_msg, ERR_MSG_LEN, "Command: %s already existed!\n", _cmd_all[_cmd_index]);
        memset(&_my_cmd[_cmd_index], 0, sizeof(_my_cmd[_cmd_index]));
        memset(_cmd_all[_cmd_index], 0, sizeof(_cmd_all[_cmd_index]));
        return -1;
    }
    
    _cmd_index++;
    return (_cmd_index - 1);
}

/***************************************************************/
/**函  数：void lshell_set_promt(const char *pmt) ***************/
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
/**函  数：void lshell_set_errmsg_swtich(int flag) **************/
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
/**函  数：void lshell_init() ***********************************/
/* 说  明：lshell初始化 ******************************************/
/* 参  数：无 ***************************************************/
/* 返回值：无 ***************************************************/
/***************************************************************/
void lshell_init()
{
    lshell_readline_init();
    lshell_set_promt("lshell");

    return;
}

/****************************************************************/
/**函  数：void lshell_start() ***********************************/
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
