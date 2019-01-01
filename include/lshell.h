#ifndef _LSHELL_H_
#define _LSHELL_H_

void lshell_init();
void lshell_set_promt(const char *str);
int lshell_register(int parent, const char *cmd, const char *tip, void (* func)(int argc, char **argv));
void lshell_start();

#endif
