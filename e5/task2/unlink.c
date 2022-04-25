int main(int argc, char const *argv[])
{
  shm_unlink("/shared_mem"); // delete shared memory
  return 0;
}
