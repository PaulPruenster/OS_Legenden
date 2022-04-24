#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

struct data
{
  int fd;
  uint64_t *shared_mem;
};

struct data *allocate_ring_buff(const char *name, uint64_t b)
{

  const int oflag = O_CREAT | O_EXCL | O_RDWR;

  const mode_t permission = S_IRUSR | S_IWUSR; // 600
  const int fd = shm_open(name, oflag, permission);
  if (fd < 0)
  {
    perror("shm_open");
    return NULL;
  }

  const size_t shared_mem_size = b * sizeof(uint64_t);

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
  struct data *structdata = malloc(sizeof(structdata));
  structdata->fd = fd;
  structdata->shared_mem = shared_mem;

  return structdata;
}

uint64_t reader(uint64_t n, uint64_t b, struct data *structdata)
{
  uint64_t sum = 0;
  for (uint64_t i = 0; i < n; ++i)
  {
    if (n > b)
    {
      sum += structdata->shared_mem[i % b];
    }
    else
    {
      sum += structdata->shared_mem[i];
    }
  }
  return sum;
}

void writer(uint64_t n, uint64_t b, struct data *structdata)
{
  for (uint64_t i = 0; i < n; ++i)
  {
    if (n > b)
    {
      structdata->shared_mem[i % b] = i + 1;
    }
    else
    {
      structdata->shared_mem[i] = i + 1;
    }
  }
  // usleep(200 * 1000); // Give reader a chance to read the message
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("usage ./<filename> <N> <B>; replace N and B with int(s)");
    return EXIT_FAILURE;
  }

  char *end1 = NULL;
  char *end2 = NULL;

  uint64_t n = strtol(argv[1], &end1, 10);
  uint64_t b = strtol(argv[2], &end2, 10);

  const char *name = "/csaz9802shared_memoryasass";
  struct data *structdata = allocate_ring_buff(name, b);

  const pid_t writer_proc = fork();
  if (writer_proc == -1)
    return EXIT_FAILURE;

  if (writer_proc == 0)
  {
    writer(n, b, structdata);
    exit(0);
  }

  const pid_t reader_proc = fork();
  if (reader_proc == -1)
    return EXIT_FAILURE;

  if (reader_proc == 0)
  {
    printf("%lu", reader(n, b, structdata));
    exit(0);
  }
  wait(0);

  munmap(structdata->shared_mem, b * sizeof(uint64_t));
  close(structdata->fd);
  free(structdata);
  shm_unlink(name);
}

/*	Observations:
 *  -> If you quit your program clean (close and unlink the file/shared memory segment)
 *     you should get the correct answer. But there is a problem, if N > B you have
 *     to use the ring buffer. Due to the implementation, the ring buffer doesn't work
 *     as expected, because it doesn't wait for the reader to read the value and then
 *     override it. It overrides it, nevertheless the reader has read it or not.
 *
 *  -> If you try really large numbers for N and B, it is likely to run into an overflow or even the
 *     program will crash, due to a too big number and end in an SIGSEV.
 *
 */
