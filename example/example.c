#include <stdio.h>

#include "lshell.h"

void test_cd(int argc, char **argv)
{
	int i;
	
	printf("cd\n");
	printf("%d\n", argc);
	for(i = 0; i < argc; i++)
	{
		printf("%s\n", argv[i]);
	}
	
	return;
}

void test_ls(int argc, char **argv)
{
	int i;
	
	printf("ls\n");
	printf("%d\n", argc);
	for(i = 0; i < argc; i++)
	{
		printf("%s\n", argv[i]);
	}
	
	return;
}

int main(int argc, char **argv)
{
	int ret;
	
	lshell_init();
	lshell_set_promt("ex");
	lshell_set_errmsg_swtich(1);
	ret = lshell_register(-1, "ls", "ls", test_ls, RUN_AT_MAIN_THREAD);
	lshell_register(ret, "cd", "cd", test_cd, RUN_AT_MAIN_THREAD);
	lshell_start();
		
	return 0;		
}
