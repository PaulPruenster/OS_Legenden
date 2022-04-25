#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>


void writer(uint64_t n, uint64_t b, uint64_t shared_mem_size, const char * name){

	const int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);

	if (fd < 0) {
		perror("shm_open");
		return;
	}

	uint64_t * shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (shared_mem == MAP_FAILED){
		perror("mmap");
		return;
	}

	for (uint64_t i = 0; i < n; ++i) {
		if (n >= b){
			shared_mem[i % b] = i + 1;
		} else{
			shared_mem[i] = i + 1;
		}
	}

	munmap(shared_mem, shared_mem_size);
	close(fd);
}

uint64_t reader(uint64_t n, uint64_t b, uint64_t shared_mem_size, const char * name){
	const int fd = shm_open(name, O_RDWR,0);
	if (fd < 0) {
		perror("shm_open");
		return EXIT_FAILURE;
	}

	uint64_t * shared_mem = mmap(NULL,  shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED){
		perror("mmap");
		return EXIT_FAILURE;
	}

	uint64_t sum = 0;
	for (uint64_t i = 0; i < n; ++i) {
		if (n >= b){
			sum += shared_mem[i % b];
		} else{
			sum += shared_mem[i];
		}
	}
	munmap(shared_mem, shared_mem_size);
	close(fd);
	return sum;
}

int initialize_shared_mem(const char * name, const uint64_t shared_mem_size){
	const int oflag = O_CREAT | O_EXCL | O_RDWR;    //fail if name already exists, read+write
	const mode_t permission = S_IRUSR | S_IWUSR;    //600

	const int fd = shm_open(name, oflag, permission);
	if (fd < 0) {
		perror("shm_open");
		return EXIT_FAILURE;
	}

	if (ftruncate(fd, shared_mem_size) != 0){
		perror("ftruncate");
		return EXIT_FAILURE;
	}

	uint64_t * shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED){
		perror("mmap");
		return EXIT_FAILURE;
	}

	munmap(shared_mem, shared_mem_size);
	close(fd);
	return EXIT_SUCCESS;
}

int main (int argc, char * argv[]){

	if (argc != 3){
		return EXIT_FAILURE;
	}

	char * end1 = NULL;
	char * end2 = NULL;

	uint64_t n = strtol(argv[1], &end1, 10);
	uint64_t b = strtol(argv[2], &end2, 10);

	char * name = "/shared_mem";
	const uint64_t shared_mem_size = b * sizeof(uint64_t);

	if(initialize_shared_mem(name, shared_mem_size) != EXIT_SUCCESS){ 
		return EXIT_FAILURE;
	}

	const pid_t writer_proc = fork();
	if(writer_proc == -1){
		return EXIT_FAILURE;
	}

	if(writer_proc == 0){
		writer(n, b, shared_mem_size, name);
		exit(0);
	}


	const pid_t reader_proc = fork();
	if(reader_proc == -1) {
		return EXIT_FAILURE;
	}

	if(reader_proc == 0) {
		printf("%llu", reader(n, b, shared_mem_size, name));
		exit(0);
	}

	wait(NULL); // wait for both forks
	wait(NULL);

	shm_unlink(name); //delete shared memory

	return EXIT_SUCCESS;
}