#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct data {
	int fd;
	uint64_t * shared_mem;
};

void* allocate_ring_buff(const char * name, uint64_t b, int mode){

	int oflag = 0;
	if (mode == 1) { // reader
		oflag = O_RDWR;
	} else if (mode == 2){ // writer
		oflag = O_CREAT | O_EXCL | O_RDWR; // create, fail if exists, read+write
	}
	const mode_t permission = S_IRUSR | S_IWUSR; // 600
	const int fd = shm_open(name, oflag, permission);
	if(fd < 0) {
		perror("shm_open");
		return NULL;
	}

	const size_t shared_mem_size = b * sizeof(uint64_t);
	if(ftruncate(fd, shared_mem_size) != 0) {
		perror("ftruncate");
		return NULL;
	}

	uint64_t * shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(shared_mem == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	struct data structdata = { .fd = fd, .shared_mem=shared_mem};
	return &structdata;
}
uint64_t reader(uint64_t n, uint64_t b) {
	const char* name = "/csaz9802shared_memoryas";
	struct data * storage = allocate_ring_buff(name, b, 2);

	if(storage == NULL){
		printf("ERROR while allocating");
		return -1;
	}

	//uint64_t * buffer = calloc(b, shared_mem_size);
	//memcpy(buffer, shared_mem, shared_mem_size);

	uint64_t sum = 0;
	for (uint64_t i = 0; i < n; ++i) {
		if (n > b){
			sum += storage->shared_mem[i % b];
		} else{
			sum += storage->shared_mem[i];
		}
	}
	munmap(storage->shared_mem, b * sizeof(uint64_t));
	close(storage->fd);
	return sum;
}

void writer(uint64_t n, uint64_t b) {
	const char* name = "/csaz9802shared_memoryas";
	struct data * storage = allocate_ring_buff(name, b, 2);

	if(storage == NULL){
		printf("ERROR while allocating");
		return;
	}

	//uint64_t * buffer = calloc(b, shared_mem_size);
	//memcpy(buffer, shared_mem, shared_mem_size);

	for (uint64_t i = 0; i < n; ++i) {
		if (n > b){
			storage->shared_mem[i % b] = i + 1;
		} else{
			storage->shared_mem[i] = i + 1;
		}
	}
	usleep(200 * 1000); // Give reader a chance to read the message
	munmap(storage->shared_mem, b * sizeof(uint64_t));
	close(storage->fd);
	shm_unlink(name);
}

int main(int argc, char** argv) {
	if (argc != 3){
		printf("usage ./<filename> <N> <B>; replace N and B with int(s)");
		return EXIT_FAILURE;
	}

	char * end1 = NULL;
	char * end2 = NULL;

	uint64_t n = strtol(argv[1], &end1, 10);
	uint64_t b = strtol(argv[2], &end2, 10);
	writer(n, b);
	printf("%llu", reader(n, b));
}
