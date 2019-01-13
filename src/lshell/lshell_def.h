#ifndef _LSHELL_DEF_H_
#define _LSHELL_DEF_H_

#define COMMAND_MAX_NUM 128  /* 命令的最大数量 */ 
#define COMMAND_MAX_LEN 15   /* 每条命令及参数的最大长度 */
#define COMMAND_MAX_TIP 127  /* 命令说明的最大长度 */
#define COMMAND_MAX_DEP 3    /* 命令的最大层次 */
#define PROMT_MAX_LEN   15   /* 提示符的最大长度 */
#define INPUT_MAX_LEN   2047 /* 输入的最大长度 */
#define INPUT_ARG_CNT   128  /* 输入中参数的最大个数 */
#define ARGS_MAX_LEN    127  /* 输入中参数的最大长度 */
#define ERR_MSG_LEN     127  /* 错误提示的最大长度 */

/* 结构化存储每条命令 */
typedef struct _command{
    int id;                                          /* 通常为结构体数组下标 */
    int parent;                                      /* 该命令的父命令id */
    char cmd[COMMAND_MAX_DEP * COMMAND_MAX_LEN + 1]; /* 存储该命令 */
    char tip[COMMAND_MAX_TIP + 1];                   /* 该命令的说明 */
    void (* func)(int argc, char **argv);            /* 该命令的函数指针 */
}command_s;

#endif