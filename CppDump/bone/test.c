#include <stdio.h>
#include <unistd.h>

int main() {
  int i = 0;
  // while (1) {
  //   printf("interesting, this will wokr!\nright? ");
  //   sleep(1);
  // }
  printf("interesting, this will wokr!\nright? ");
  scanf("%d", &i);
  printf("lol nerd %d\n", i);
  return 0;
}
