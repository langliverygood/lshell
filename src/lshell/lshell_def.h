#ifndef _LSHELL_DEF_H_
#define _LSHELL_DEF_H_

#define COMMAND_MAX_NUM 128  /* 命令的最大数量 */ 
#define COMMAND_MAX_LEN 2047 /* 命令的最大长度 */ 
#define COMMAND_MAX_TIP 127  /* 命令说明的最大长度 */
#define PROMT_MAX_LEN   15   /* 提示符的最大长度 */
#define INPUT_MAX_LEN   2047 /* 输入的最大长度 */
#define ERR_MSG_LEN     2047 /* 错误提示的最大长度 */

/* 结构化存储每条命令 */
typedef struct _command{
    int id;                               /* 通常为结构体数组下标 */
    int parent;                           /* 该命令的父命令id */
    char cmd[COMMAND_MAX_LEN + 1];        /* 存储该命令 */
    char tip[COMMAND_MAX_TIP + 1];        /* 该命令的说明 */
    void (* func)(int argc, char **argv); /* 该命令的函数指针 */
    int mode;                             /* 该命令的执行方式 0:在主线程中执行命令; 非0:在新线程执行命令 */
    /* 当mode!=0 时以下成员有效 */
    int join_detach;                      /* 线程的状态,0 joined 1 detached */
	int cancelstate;                      /* 0表示收到信号后设为CANCLED状态,非0忽略CANCEL信号继续运行*/
	int canceltype;                       /* 当cancelstate为0时有效，0表示收到信号后继续运行至下一个取消点再退出,1立即执行取消动作(出) */
}command_s;

/***************************************************************/
/* 说  明：预定义help命令 *****************************************/
/***************************************************************/
void lshell_help(int argc, char **argv);

/***************************************************************/
/* 说  明：预定义exit命令 *****************************************/
/***************************************************************/
void lshell_exit(int argc, char **argv);

/***************************************************************/
/* 说  明：预定义threads命令 **************************************/
/***************************************************************/
void lshell_threads(int argc, char **argv);

/***************************************************************/
/* 说  明：预定义kill令 ******************************************/
/***************************************************************/
void lshell_kill(int argc, char **argv);

/***************************************************************/
/* 函  数：start_new_thread() ***********************************/
/* 说  明：启动新线程去执行 ****************************************/
/* 参  数：_my_cmd_index 命令结构体的下标 **************************/
/*        message 线程说明信息 ***********************************/
/* 返回值：无 ****************************************************/
/***************************************************************/
void start_new_thread(int _my_cmd_index, char *message);

#endif
