#include <stdio.h>
#include <stdlib.h>

// How clipboards work is very interesting...
// Initially, there used to be 3 Clipboards:
//    1. Primary: The selection highlighted by mouse
//    2. Secondary: Not used generally. Mostly for in app
//                  specific copy paste
//    3. Clipboard: The usual <C-c> and <C-v> buffer
//
// X11 environment used to have all these 3 "types" of
// clipboard. Although Wayland does not support the
// Primary selection (or it did not use to). Although
// wl-clipboard can be used to access the currently
// highlighted text.
//
// There's no easy way to get the clipboard, one of the reason
// is because it depends on the compositor protocol. X11 has a
// different one that from Wayland. Thus need an external
// dependency (sadly).
//
// $ wl-paste -p
//
// ref:
// https://superuser.com/questions/90257/what-is-the-difference-between-the-x-clipboards

int checkPackage(const char *pkgName) {
  char cmd[512];
  // Finally! i understand tf 2>&1 means
  //
  // the `2` and `1` are the file descriptors, as usual
  // 1 -> stdout, 2 -> stderr
  // So when we do `2> file`, the stderr is written to file,
  // while `> file` and `1> file` means the same thing.
  //
  // To direct stderr to stdout we dont do `2>1`, this would
  // be considered to direct stderr into a file named `1`.
  // To specify we mean fd 1, we use `2>&1`
  sprintf(cmd, "which %s > /dev/null 2>&1", pkgName);

  int retVal = system(cmd);
  if (retVal == -1) {
    printf("system() failed to execute.");
    perror("system err: ");
    exit(1);
  }

  // The retVal is shifted 8 bits, dk why
  if (WEXITSTATUS(retVal) != 0) {
    printf("Cannot find %s in PATH\n", pkgName);
    printf("which %s returned %d\n", pkgName, WEXITSTATUS(retVal));
    return 1;
  }

  return 0;
}

int main() {
  if (checkPackage("wl-copy") || checkPackage("wl-paste")) {
    printf("please install wl-clipboard: sudo pacman -S wl-clipboard\n");
    return 1;
  }
  printf("Found wl-clipboard\n");
  printf("primary buffer contains:\n");

  FILE *fp = popen("wl-paste -p", "r");
  if (fp == NULL) {
    perror("popen error");
    return 1;
  }

  // BUFSIZ is defined in stdio.h -> default buffer size
  // 8192
  char buf[BUFSIZ];
  while (fgets(buf, BUFSIZ, fp) != NULL)
    printf("%s\n", buf);

  if (pclose(fp) == -1) {
    perror("unable to close fp");
    return 1;
  }

  return 0;
}
