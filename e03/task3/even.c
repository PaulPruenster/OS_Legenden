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