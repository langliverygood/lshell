#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <malloc.h>

#include "lshell_def.h"
#include "lshell_readline.h"

extern command_s _my_cmd[COMMAND_MAX_NUM]; /* 命令数组,下标从0开始 */
extern int _cmd_index;                     /* 当前第一个未使用的数组下标 */
extern char _promt[PROMT_MAX_LEN + 1];     /* 提示符 */
static char *_input_buf;                   /* 输入 */

/* 说明：tab键补全命令 */
char* command_generator(const char *text, int state)
{
    const char *name;
    static int list_index, len;

    if(!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while(list_index < _cmd_index)
    {
        name = _my_cmd[list_index++].cmd;
        if (strncmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }

    return ((char *)NULL);
}

/* 说明：tab键补全命令 */
char** command_completion (const char *text, int start, int end)
{
    char **matches = NULL;

    matches = rl_completion_matches(text, command_generator);

    return (matches);
}

/* 说明：readline库初始化 */
void lshell_readline_init()
{
    rl_readline_name = "lshell";
    rl_attempted_completion_function = command_completion;
    
    return;
}

/* 说明：从终端读取一次输入 */
char *lshell_gets()
{
    if(_input_buf)
    {    
        free(_input_buf);
        _input_buf = (char *)NULL;
    }
    _input_buf = readline(_promt);

    if(_input_buf && *_input_buf)
    {
        add_history(_input_buf);
    }
    
    return(_input_buf);
}
