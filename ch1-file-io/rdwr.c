#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

int main(int args,char* argv[])
{
	char buf[127] = {0};
	int cnt = 0;
	cnt = read(STDIN_FILENO, buf, 127);
	if(cnt < 0)
	{
		perror("Read STDIN_FILENO error!");
		exit(1);
	}
	write(STDOUT_FILENO, buf, cnt);
	return 0;
}


