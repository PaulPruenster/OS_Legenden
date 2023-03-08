#include <stdio.h>
#include <stdlib.h>

static char var_name[] = "MY_ENV_VAR";
int main() {
	char * var = getenv(var_name);
	if(var == NULL) {
		printf("%s is not set\n", var_name);
	} else {
		printf("%s is set to '%s'\n", var_name, var);
	}
	return EXIT_SUCCESS;
}
