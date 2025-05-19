// GNU's way to deal with directories -> dirent.h
// + a directory is also considered a file, just
// with a special type!
// This means you can use fopen/open to access them
// too!
//
// Example:
// int fd = open("/dev/input/by-path/", O_RDONLY);
// if (fd < 0) {
//   perror("Failed to open device");
//   exit(1);
// }
// printf("FD for /dev/input/by-path/: %d\n", fd);
//
//
#include <dirent.h>
// Again, just like the FILE*, directories are also
// represented as streams that one can access using
// a DIR*

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> // -> readlink

// Invaluable lesson learned:
// 1. size_t    -> use when we know we have data in bytes (0 -> SIZE_MAX)
// 2. ssize_t   -> use when we know data can either be:
//                    1. >= 0 if its valid or
//                    2. <  0 if     invalid
//                 in bytes

struct IntSet {
  int *set;
  int size;
  int capacity;
};
int initSet(struct IntSet **set, int capacity);
int pushSet(struct IntSet *set, int val);
int dynamicInc(struct IntSet *set);

int main() {
  // The `/dev/input/` directory:
  // Much like the /proc/ directory, /dev/input/ directory
  // allows us to view all the input streams as file sys.
  // One can view info like handlers, bus info, etc. w/ it
  // and even more interesting, listen to the incoming
  // inputs.
  //
  // It includes: Mouse, Keyboard, Power Button,
  // Speakers etc.

  // Create a DIR stream
  DIR *dir = opendir("/dev/input/by-path/");
  if (dir == NULL) {
    perror("Failed to open directory");
    exit(1);
  }
  printf("ref for /dev/input/by-path/ created @ %p\n", dir);

  struct IntSet *set = NULL;
  initSet(&set, 1);
  struct dirent *entry = NULL;

  const size_t BUFSIZE = 100;
  char symlinkTo[BUFSIZE], absPath[BUFSIZE];

  // readdir(DIR*) returns a ptr to the next entry in the DIR stream
  while ((entry = readdir(dir)) != NULL) {
    size_t dNameLen = strlen(entry->d_name);
    if (dNameLen < 9) {
      continue;
    }

    // In /dev/input/by-path/ symlinks ending with a `event-kbd`
    // are the symlinks that point to keyboard events
    char compVal[9];
    strncpy(compVal, entry->d_name + dNameLen - 9, 9);
    if (strcmp("event-kbd", compVal) != 0) {
      continue;
    }

    strcpy(absPath, "/dev/input/by-path/");
    strcat(absPath, entry->d_name);
    ssize_t len = readlink(absPath, symlinkTo, BUFSIZE);
    if (len == -1) {
      continue;
    }
    symlinkTo[len] = '\0';

    printf("Entry: /dev/input/by-path/%s\t\tis a symlink to -> %s\n",
           entry->d_name, symlinkTo);
    pushSet(set, atoi((symlinkTo + strlen(symlinkTo) - 1)));
  }

  closedir(dir);

  int size = set->size;
  printf("eventX is a keyboard Event | X =  ");
  while (size-- > 0) {
    printf("%d, ", set->set[size]);
  }
  printf("\b\b;\n");
}

// Dynamic Set Implementation
int initSet(struct IntSet **set, int capacity) {
  *set = (struct IntSet *)malloc(sizeof(struct IntSet));
  if (set == NULL) {
    return 1;
  }
  (*set)->size = 0;
  (*set)->capacity = capacity;
  (*set)->set = (int *)malloc(sizeof(int) * capacity);

  return 0;
}

int pushSet(struct IntSet *set, int val) {
  if (set == NULL) {
    exit(1);
  }
  int size = set->size;
  while (size-- > 0) {
    if (val == set->set[size]) {
      return 1;
    }
  }

  if (set->size >= set->capacity) {
    dynamicInc(set);
  }
  set->set[set->size++] = val;

  return 0;
}

// A basic dynamic array implementation
int dynamicInc(struct IntSet *set) {
  int newCapacity = set->capacity * 2;
  int *newArr = (int *)malloc(sizeof(int) * newCapacity);
  if (newArr == NULL) {
    return 1;
  }

  int size = set->size;
  while (size-- >= 0) {
    newArr[size] = set->set[size];
  }

  set->capacity = newCapacity;
  free(set->set);
  set->set = newArr;
  return 0;
}
