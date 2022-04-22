#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>

// TODO: Test on ZID

void reader(uint64_t n, uint64_t b)
{
  const char *name = "/asdf";
  const int oflag = O_RDWR; // open read+write
  const int fd = shm_open(name, oflag, 0);
  if (fd < 0)
  {
    // This fails on compiler explorer, but works on a normal system
    perror("shm_open");
    return;
  }

  const uint64_t shared_mem_size = n;
  uint64_t *shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shared_mem == MAP_FAILED)
  {
    perror("mmap");
    return;
  }

  uint64_t *buffer = calloc(b, sizeof(uint64_t) * shared_mem_size);
  memcpy(buffer, shared_mem, shared_mem_size * sizeof(uint64_t));

  munmap(shared_mem, shared_mem_size);
  close(fd);
  shm_unlink(name);

  uint64_t sum = 0;
  for (uint64_t i = 0; i < n; ++i)
  {
    if (n > b)
    {
      sum += buffer[i % b];
    }
    else
    {
      sum += buffer[i];
    }
  }
  printf("Result: %ld\n", sum);
  free(buffer);
}

void writer(uint64_t n, uint64_t b)
{
  const char *name = "/asdf";                  // change the name if it already exists
  const int oflag = O_CREAT | O_EXCL | O_RDWR; // create, fail if exists, read+write
  const mode_t permission = S_IRUSR | S_IWUSR; // 600
  const int fd = shm_open(name, oflag, permission);
  if (fd < 0)
  {
    perror("shm_open");
    return;
  }

  const uint64_t shared_mem_size = b;
  if (ftruncate(fd, shared_mem_size * sizeof(uint64_t)))
  {
    perror("ftruncate");
    return;
  }

  uint64_t *shared_mem = mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shared_mem == MAP_FAILED)
  {
    perror("mmap");
    return;
  }

  uint64_t *content = calloc(n, sizeof(uint64_t) * n);
  for (uint64_t i = 0; i < n; ++i)
  {
    if (n > b)
    {
      content[i % b] = i + 1;
    }
    else
    {
      content[i] = i + 1;
    }
  }
  memcpy(shared_mem, content, shared_mem_size * sizeof(uint64_t));

  sleep(200 * 1000 * 1000); // Give reader a chance to read the message

  munmap(shared_mem, shared_mem_size);
  close(fd);
  shm_unlink(name);
  free(content);
}

int main(int argc, char *argv[])
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

  pid_t writer_proc = fork();
  if (writer_proc == -1)
    return EXIT_FAILURE;

  if (writer_proc == 0)
  {
    writer(n, b);
    return EXIT_SUCCESS;
  }

  pid_t reader_proc = fork();
  if (writer_proc == -1)
    return EXIT_FAILURE;

  if (reader_proc == 0)
  {
    reader(n, b);
    return EXIT_SUCCESS;
  }

  wait(NULL);
  return EXIT_SUCCESS;
}
