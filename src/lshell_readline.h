#ifndef _LSHELL_READLINE_H_
#define _LSHELL_READLINE_H_

/* 说明：tab键补全命令 */
char* command_generator(const char *text, int state);

/* 说明：tab键补全命令 */
char** command_completion (const char *text, int start, int end);

/* 说明：readline库初始化 */
void lshell_readline_init();

/* 说明：从终端读取一次输入 */
char *lshell_gets();

#endif