#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  uint64 free = freemem();

  printf("Free memory: %ld bytes\n", free);

  exit(0);
}
