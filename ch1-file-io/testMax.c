#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>


int main(int args, char* argv[])
{
	//printf("Test Unix API open function\n");
	int index = 0;
	char name[1024] = {0};
	int rt = 0;
	while(1)
	{
		sprintf(name, "file%04d", ++index);
		if(-1 == open(name, O_CREAT, 0777))
		{
			printf("error!\n");
			exit(1);
		}
	}		


	return 0;
}
