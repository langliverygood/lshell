#ifndef _THREADS_MANAGE_H_
#define _THREADS_MANAGE_H_

#define THREAD_MAX_NUM 128   /* 子线程最大数量 */
#define COMMAND_MAX_LEN 2047 /* 命令的最大长度 */ 

/* 线程管理头部 */
typedef struct _thread_header{
    pthread_t tid;                                               /* 线程标识符 */
    int join_detach;                                             /* 线程的状态,0 :joined 1:detached */
    int cancelstate;                                             /* 0表示收到信号后设为CANCLED状态,非0忽略CANCEL信号继续运行*/
    int canceltype;                                              /* 当cancelstate为0时有效，0表示收到信号后继续运行至下一个取消点再退出,1立即执行取消动作（退出）*/
    uint32_t starttime_high;                                     /* 线程启动时间的高位 */
    uint32_t starttime_low;                                      /* 线程启动时间的低位 */
    uint32_t used;                                               /* 该结构体是否有效 */
    void (* func)(int argc, char **argv);                        /* 该线程启动后执行的函数 */
    uint32_t argc;                                               /* func的参数 */
    char *argv[COMMAND_MAX_LEN + 1];                             /* func的参数 */
    char argv_storage[COMMAND_MAX_LEN + 1][COMMAND_MAX_LEN + 1]; /* 由于lshell申请的参数空间要释放,所以子线程要备份 */
    char message[128];                                           /* 该线程的信息 */
}thread_header_s;

/***************************************************************/
/* 函  数：thread_start ****************************************/
/* 说  明：线程启动函数 ****************************************/
/* 参  数：th_header 线程管理头部 ******************************/
/*        message 线程信息 *************************************/
/* 返回值：0 启动成功 ******************************************/
/*      ：1 启动失败 *******************************************/
/***************************************************************/
char thread_start(thread_header_s *th_header, char *message);

/***************************************************************/
/* 函  数：thread_stop *****************************************/
/* 说  明：线程中止函数 ****************************************/
/* 参  数：th_header 线程管理头部 ******************************/
/* 返回值：0 中止成功 ******************************************/
/*      ：1 中止失败 *******************************************/
/***************************************************************/
char thread_stop(thread_header_s *th_header);

#endif
