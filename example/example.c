#include <stdio.h>

#include "lshell.h"

void ex_show(int argc, char **argv)
{
	int i;
	
	printf("This func is : ex_show\n");
	printf("argc: %d\n", argc);
	printf("argv:");
	for(i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
	
	return;
}

void ex_show_argc(int argc, char **argv)
{
	printf("This func is : ex_show_argc\n");
	printf("argc: %d\n", argc);
	
	return;
}

void ex_show_argv(int argc, char **argv)
{
	int i;
	
	printf("This func is : ex_show_argv\n");
	printf("argv:");
	for(i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
	
	return;
}

int main(int argc, char **argv)
{
	int ret;
	
	lshell_init();
	lshell_set_promt("ex");
	lshell_set_errmsg_swtich(1);
	ret = lshell_register(-1, "show", "show argc and argv", ex_show, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_register(ret, "argc", "show argc", ex_show_argc, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_register(ret, "argv", "show argv", ex_show_argv, RUN_AT_MAIN_THREAD, 0, 0, 0);
	lshell_start();
		
	return 0;		
}
