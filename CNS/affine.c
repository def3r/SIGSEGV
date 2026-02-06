#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M 26

int mod_inv(int a);
void encrypt(int a, int b, char *text);
void decrypt(int a, int b, char *text);

int main(int argc, char *argv[]) {
  if (argc != 4) {
    return 1;
  }

  int a = atoi(argv[1]);
  int b = atoi(argv[2]);
  char *plain_text = argv[3];

  printf("%d, %d\n", a, b);
  printf("Text: %s\n", plain_text);

  encrypt(a, b, plain_text);
  printf("Cipher: %s\n", plain_text);

  decrypt(a, b, plain_text);
  printf("Decrypt: %s\n", plain_text);

  return 0;
}

void encrypt(int a, int b, char *text) {
  size_t len = strlen(text);
  for (int i = 0; i < len; i++) {
    if (text[i] == ' ')
      continue;
    text[i] = 'a' + (((a * (text[i] - 'a')) + b) % M);
  }
}

void decrypt(int a, int b, char *text) {
  size_t len = strlen(text);
  int a_inv = mod_inv(a);
  for (int i = 0; i < len; i++) {
    if (text[i] == ' ')
      continue;

    // Tf is 'a' - 'A' + 1 ???
    // https://www.codespeedy.com/implementation-of-affine-cipher-in-cpp/
    text[i] = 'a' + ((a_inv * (text[i] + 'a' - 'A' + 1 - b) % 26));
  }
}

int mod_inv(int a) {
  int a_inv = 0;
  for (int i = 0; i < M; i++) {
    int flag = (a * i) % M;
    if (flag == 1)
      a_inv = i;
  }
  return a_inv;
}
