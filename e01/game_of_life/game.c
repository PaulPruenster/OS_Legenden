#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printUsage(const char* programName) {
	printf("usage: %s <width> <height> <density> <steps>\n", programName);
}

int* initArray(const int height, const int width) {
	return malloc(width * height * sizeof(int));
}
/*
smalles density is 0.001
*/
void generateField(int* field, const int width, const int height, const double density) {
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++) {
			field[i * height + j] = density * 1000 > rand() % 1000 ? true : false;
		}
	}
}

void generateNextStep(int* source, int* destination, const int width, const int height) {
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++) {
			int counter = 0;
			if(i != 0) {
				counter += source[(i - 1) * height + j]; // Above
			}
			if(j != 0) {
				counter += source[i * height + j - 1]; // Left
			}
			if(i != width - 1) {
				counter += source[(i + 1) * height + j]; // Under
			}
			if(j != height - 1) {
				counter += source[i * height + j + 1]; // Right
			}
			if(j != 0 && i != 0) {
				counter += source[(i - 1) * height + j - 1]; // Left Above
			}
			if(i != width - 1 && j != 0) {
				counter += source[(i + 1) * height + j - 1]; // Left Unten
			}
			if(j != height - 1 && i != 0) {
				counter += source[(i - 1) * height + j + 1]; // Right Above
			}
			if(i != width - 1 && j != height - 1) {
				counter += source[(i + 1) * height + j + 1]; // Right Under
			}
			if((source[i * height + j] == false && counter == 3) ||
			   (source[i * height + j] == true && counter >= 2 && counter <= 3)) {
				destination[i * height + j] = true;
			} else {
				destination[i * height + j] = false;
			}
		}
	}
}
/*
!Warnung! used differently in rest of code
0 = alive
1 = dead
*/
void generatePBM(int* field, const int width, const int height, char* name) {
	for(int i = 0; i < 1; i++) {
		FILE* pgmimg;
		pgmimg = fopen(name, "wb");
		// name is made with
		// malloc, so a free
		// is needed
		free(name);
		fprintf(pgmimg, "P1\n");
		fprintf(pgmimg, "%d %d\n", width, height);
		for(int i = 0; i < width; i++) {
			for(int j = 0; j < height; j++) {
				// changes 1 to 0 and 0 to 1
				fprintf(pgmimg, "%d", !field[i * height + j]);
			}
		}
		fclose(pgmimg);
	}
}

char* getPBMname(const int zahl) {
	char* name = malloc(14 * sizeof(char));
	snprintf(name, 14, "gol_%05d.pbm", zahl);
	return name;
}

int main(int argc, char* argv[]) {
	if(argc != 5) {
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}

	const int width = atoi(argv[1]);
	const int height = atoi(argv[2]);
	const double density = atof(argv[3]);
	const int steps = atoi(argv[4]);

	printf("width:   %4d\n", width);
	printf("height:  %4d\n", height);
	printf("density: %4.0f%%\n", density * 100);
	printf("steps:   %4d\n", steps);

	// Seeding the random number generator so we get a different
	// starting field every time.
	srand(time(NULL));
	int* source = NULL;
	int* destination = NULL;
	if(steps > 0) {
		source = initArray(height, width);
		destination = initArray(height, width);
		generateField(source, width, height, density);
		generatePBM(source, width, height, getPBMname(0));
	}

	for(int i = 1; i < steps; i++) {
		generateNextStep(source, destination, width, height);
		generatePBM(destination, width, height, getPBMname(i));
		int* temp = source;
		source = destination;
		destination = temp;
	}

	free(source);
	free(destination);
	return EXIT_SUCCESS;
}
