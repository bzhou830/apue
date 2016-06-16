#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

int main(int args, char* argv[])
{
	printf("Test Unix API open function\n");
	int rt = open("open.txt", O_CREAT, 0777);
	if(rt == -1)
	{
		printf("open file error!\n");
		exit(1);
	}
	return 0;
}
