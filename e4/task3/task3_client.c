// C program to implement one side of FIFO
// This side reads first, then reads
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("wrong amount of arguments!");
		return EXIT_FAILURE;
	}
	char name[strlen(argv[1] + 5)];
	sprintf(name, "/tmp/%s", argv[1]);
	int fd1;

	char str1[80];
	int finish = 1;
	fd1 = open(name, O_WRONLY);
	while (finish)
	{
		// string taken from user.
		printf("Expression: ");
		// scanf("%s", str1);
		fgets(str1, 1000, stdin);
		if ((strlen(str1) > 0) && (str1[strlen(str1) - 1] == '\n'))
			str1[strlen(str1) - 1] = '\0';
		// printf("%s", str1);
		if (str1[0] == '\0')
		{
			finish = 0;
			close(fd1);
		}
		else
		{
			write(fd1, str1, strlen(str1));
			// close(fd1);
		}
	}
	return 0;
}