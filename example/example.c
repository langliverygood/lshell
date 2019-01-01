#include <stdio.h>

#include "lshell.h"

void test_qqq(int argc, char **argv)
{
	int i;
	
	printf("qqq\n");
	printf("%d\n", argc);
	for(i = 0; i < argc; i++)
	{
		printf("%s\n", argv[i]);
	}
	
	return;
}

int main(int argc, char **argv)
{
	lshell_init();
	lshell_set_promt("test");
	lshell_register(-1, "qqq", "qqq", test_qqq);
	lshell_start();
		
	return 0;		
}
