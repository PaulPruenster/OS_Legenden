#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

#define WRITE 0
#define CONSUME 1

typedef struct data
{
  int fd;
  uint64_t *shared_mem;
  uint64_t shared_mem_size;
} ThreadData;

typedef struct sema
{
  int fd;
  sem_t *shared_mem;
} SemaData;

void writer(uint64_t n, uint64_t b, ThreadData *structdata, SemaData *semadata)
{
  for (uint64_t i = 0; i < n; ++i)
  {
    sem_wait(&semadata->shared_mem[WRITE]);
    if (n >= b)
      structdata->shared_mem[i % b] = i + 1;
    else
      structdata->shared_mem[i] = i + 1;
    sem_post(&semadata->shared_mem[CONSUME]);
  }

  munmap(structdata->shared_mem, structdata->shared_mem_size);
  close(structdata->fd);
}

uint64_t reader(uint64_t n, uint64_t b, ThreadData *structdata, SemaData *semadata)
{
  uint64_t sum = 0;
  for (uint64_t i = 0; i < n; ++i)
  {
    sem_wait(&semadata->shared_mem[CONSUME]);
    if (n >= b)
      sum += structdata->shared_mem[i % b];
    else
      sum += structdata->shared_mem[i];
    sem_post(&semadata->shared_mem[WRITE]);
  }
  munmap(structdata->shared_mem, structdata->shared_mem_size);
  close(structdata->fd);
  return sum;
}

void *initialize_shared_mem(const char *name, const uint64_t shared_mem_size)
{
  const int oflag = O_CREAT | O_EXCL | O_RDWR; // fail if name already exists, read+write
  const mode_t permission = S_IRUSR | S_IWUSR; // 600

  const int fd = shm_open(name, oflag, permission);
  if (fd < 0)
  {
    perror("shm_open");
    return NULL;
  }

  if (ftruncate(fd, shared_mem_size) != 0)
  {
    perror("ftruncate");
    return NULL;
  }

  uint64_t *shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shared_mem == MAP_FAILED)
  {
    perror("mmap");
    return NULL;
  }
  ThreadData *structdata = malloc(sizeof(structdata));
  structdata->fd = fd;
  structdata->shared_mem = shared_mem;
  structdata->shared_mem_size = shared_mem_size;
  return structdata;
}

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    return EXIT_FAILURE;
  }

  char *end1 = NULL;
  char *end2 = NULL;

  uint64_t n = strtol(argv[1], &end1, 10);
  uint64_t b = strtol(argv[2], &end2, 10);

  char *name_sem = "/shared_mem_sem";
  const int shared_mem_size_sem = sizeof(sem_t) * 2;
  SemaData *semadata = initialize_shared_mem(name_sem, shared_mem_size_sem);
  if (semadata == NULL)
  {
    return EXIT_FAILURE;
  }
  sem_init(&semadata->shared_mem[WRITE], 1, 1);
  sem_init(&semadata->shared_mem[CONSUME], 1, 0);

  char *name = "/shared_mem";
  const uint64_t shared_mem_size = b * sizeof(uint64_t);
  ThreadData *structdata = initialize_shared_mem(name, shared_mem_size);
  if (structdata == NULL)
  {
    return EXIT_FAILURE;
  }

  const pid_t writer_proc = fork();
  if (writer_proc == -1)
  {
    return EXIT_FAILURE;
  }

  if (writer_proc == 0)
  {
    writer(n, b, structdata, semadata);
    exit(0);
  }

  const pid_t reader_proc = fork();
  if (reader_proc == -1)
  {
    return EXIT_FAILURE;
  }

  if (reader_proc == 0)
  {
    printf("%llu", reader(n, b, structdata, semadata));
    exit(0);
  }

  wait(NULL); // wait for both forks

  sem_destroy(&semadata->shared_mem[WRITE]);
  sem_destroy(&semadata->shared_mem[CONSUME]);
  munmap(structdata->shared_mem, shared_mem_size);
  close(structdata->fd);
  shm_unlink(name);     // delete shared memory
  shm_unlink(name_sem); // delete shared memory

  return EXIT_SUCCESS;
}
/*	Observations:
 *  -> If you quit your program clean (close and unlink the file/shared memory segment)
 *     you can get the correct answer if you are a lucky and n < b. But there is a problem, if N > B you have
 *     to use the ring buffer. Due to the implementation, the ring buffer doesn't work
 *     as expected, because it doesn't wait for the reader to read the value and then
 *     overrides it. It overrides it, nevertheless the reader has read it or not.
 *
 *  -> If you try really large numbers for N and B, it is likely to run into an overflow or even the
 *     program will crash, due to a too big number and end in an SIGSEV.
 *
 */