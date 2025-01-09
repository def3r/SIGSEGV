#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX 5000

typedef struct CLArgs {
  int curr;
  int argc;
  char **argv;
} CLArgs;

typedef struct Count {
  int lines;
  int words;
  int chars;
} Count;

typedef struct Flags {
  bool flagFound;
  bool l; // lines
  bool w; // words
  bool m; // chars
} Flags;

bool isDelim(char *c) {
  if (c[0] == ' ')
    return true;
  if (c[0] == '\t')
    return true;
  if (c[0] == '\n')
    return true;
  return false;
}

void setFlags(CLArgs args, Flags *f) {
  if (args.curr >= args.argc)
    return;
  if (args.argv[args.curr][0] != '-') {
    args.curr++;
    return setFlags(args, f);
  }

  char *p = &args.argv[args.curr][1];
  while (p[0]) {
    if (p[0] == 'l') {
      f->l = true;
    } else if (p[0] == 'w') {
      f->w = true;
    } else if (p[0] == 'm') {
      f->m = true;
    } else {
      printf("Unknown flag %s", args.argv[args.curr]);
      exit(1);
    }
    p++;
  }
  f->flagFound = true;
  args.curr++;
  return setFlags(args, f);
}

void calculate(FILE *f, Count *count) {
  char bffr[MAX];
  while (fgets(bffr, MAX, f)) {
    char *f = &bffr[0];
    char *r = f;
    while (f) {
      count->chars++;
      if (f[0] == '\n') {
        if (!isDelim(r)) {
          count->words++;
        }
        break;
      }
      if (f[0] == ' ' && !isDelim(r)) {
        count->words++;
      }
      r = f;
      f++;
    }
    count->lines++;
  }
}

void display(Flags flags, Count count) {
  if (flags.l || !flags.flagFound) {
    printf("%d\t", count.lines);
  }
  if (flags.w || !flags.flagFound) {
    printf("%d\t", count.words);
  }
  if (flags.m || !flags.flagFound) {
    printf("%d\t", count.chars);
  }
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("No file provided");
    return 1;
  }

  FILE *f;
  Flags flags = {.l = false, .w = false, .m = false};
  setFlags((CLArgs){.curr = 1, .argc = argc, .argv = argv}, &flags);

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      continue;
    }
    f = fopen(argv[i], "r");
    if (!f) {
      printf("Can't find the file %s", argv[i]);
      return 1;
    }
    Count count = {.lines = 0, .words = 0, .chars = 0};
    calculate(f, &count);
    display(flags, count);
    printf("%s\n", argv[i]);
  }
  return 0;
}
