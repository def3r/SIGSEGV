#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define println(...)                                                           \
  printf(__VA_ARGS__);                                                         \
  printf("\n");

// interestingly, the file system is way more complex
// than it looks
//
// A file’s name is distinct from the file itself; the same underlying file, ca
// lled an inode, can have multiple names, called links. The fstat system call
// retrieves information from the inode that a file descriptor refers to. The
// link system call creates another file system name referring to the same inode
// as an exist-ing file.

int main() {
  char c;
  int fd = open("abc.txt", O_CREAT | O_WRONLY, 0644);

  println("Opened abc.txt, you can see this file.");
  scanf("%c", &c);
  close(fd);

  // create the file but then unlink the name from its inode
  // the file now exists but will be cleaned up once the fd
  // is closed or goes out of scope!
  fd = open("abc.txt", O_CREAT | O_WRONLY, 0644);
  unlink("abc.txt");

  println("Opened abc.txt, you can not see this file.");
  println("abc");
  scanf("%c", &c);
  close(fd);

  return EXIT_SUCCESS;
}
