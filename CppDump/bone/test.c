#include <stdio.h>
#include <unistd.h>

int main() {
  int i = 0;
  // while (1) {
  //   printf("interesting, this will wokr!\nright? ");
  //   sleep(1);
  // }
  printf("test ohoo 123 123!\nright? ");
  scanf("%d", &i);
  printf("well, :D nerd %d\n", i);
  scanf("%d", &i);
  while (1) {
    printf("|");
  }
  return 0;
}
