#include <stdio.h>
#include <string.h>

/*  proc FileSystem
 *
 *  The low level, kernel and sys related information
 *  can be accessed through a *virtual* filesystem wh
 *  ich is proc!
 *
 *    - Contain Real Time Information!
 *
 *    - Does not exist on the disk! Snap of Kernel at
 *      the time of invocation.
 *
 *    - Contain the pid and t(hread)id subdirectories
 *      that store the metadata and kernal
 *      related information about that specific pro
 *      cess.
 */

int main() {
  FILE *f = NULL;
  char bffr[1024];
  int lines = 0;
  int coreCPU = 0;

  printf("Waiting for /proc/meminfo ...");
  f = fopen("/proc/meminfo", "r");
  printf("\033[2K\r");

  printf("Memory Information:\n");
  while (lines++ < 3 && fgets(bffr, sizeof(bffr), f)) {
    printf("%s", bffr);
  }

  printf("\n");
  fclose(f);

  printf("Waiting for /proc/stat ...");
  f = fopen("/proc/stat", "r");
  printf("\033[2K\r");

  printf("Total CPU Cores: ");
  while (fgets(bffr, sizeof(bffr), f)) {
    char bffrCheck[4] = "";
    strncpy(bffrCheck, bffr, 3);
    bffrCheck[3] = '\0';

    if (strcmp(bffrCheck, "cpu") != 0) {
      coreCPU--; // First cpu is THE CPU not core (idk something like that is
                 // suppose)
      break;
    }
    coreCPU++;
  }
  printf("%d\n", coreCPU);

  fclose(f);
  return 0;
}
