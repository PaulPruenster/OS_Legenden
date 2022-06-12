#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("wrong number of arguments\n");
        return EXIT_FAILURE;
    }

    long a; 
    char * ptr1; 
    a = strtol(argv[1], &ptr1, 10);
    if (*ptr1 != '\0'){
        printf("error\n");
        return EXIT_FAILURE;
    }
    printf("%ld", a);


}
//___________________________________________________________________

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {

	if(argc != 2) return 2;
	long int n = atoi(argv[1]);
	if((n == 0 && argv[1][0] != '0')) return 3;
	return n % 2;
}