#ifndef _LSHELL_H_
#define _LSHELL_H_

typedef void (* lshell_func)(int argc, char **argv);

void lshell_init();
void lshell_set_promt(const char *str);
int lshell_register(int parent, const char *cmd, const char *tip, lshell_func func);
void lshell_start();

#endif
