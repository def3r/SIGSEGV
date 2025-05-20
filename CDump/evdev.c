// Check evdev.md for explanation & References
//
// Need to Link with libevdev
// $ gcc evdev.c -o evdev.out -L/usr/local/lib -levdev
// $ sudo ./evdev.out
//
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// <libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev.h>

// for suspending until an event is encountered
#include <sys/select.h>
// Also: select is old fashioned. According to the manual,
// "modern applications" should not use it because it
// may not suffice its need. `select` can only hold upto
// 1024 file descriptors in its set, thus one can not
// rely on select to monitor more than 1024 fds.
//
// Alternative: poll() or epoll()

const size_t BUFSIZE = 100;

struct IntSet {
  int *set;
  int size;
  int capacity;
};
int initSet(struct IntSet **set, int capacity);
int pushSet(struct IntSet *set, int val);
int dynamicInc(struct IntSet *set);
int intLen(int n);

int getKbdEvents(struct IntSet **set);

int main() {
  struct libevdev *dev = NULL;
  struct IntSet *set = NULL;

  getKbdEvents(&set);
  printf("------\n");

  char kbd[BUFSIZE];
  int fds[set->size], fd;
  struct libevdev *devs[set->size];
  for (int i = 0; i < set->size; i++) {
    snprintf(kbd, 18 + intLen(set->set[i]), "/dev/input/event%d", set->set[i]);
    printf("Opening: %s\n", kbd);

    // Open the eventX as any other file and get it an fd
    // Altho it is readonly in a non-blocking fashion
    fd = open(kbd, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
      perror("Failed to open device");
      return 1;
    }

    // So from what i understand of libevdev, libevdev_new_from_fd
    // is kind of `subscribing` to the event device.
    // Like an event listener to the event device.
    // It does not create a new device as i was very wrong
    // in thinking that was the case.
    //
    // Use the already existing fd to, kind of, `subscribe` to its
    // events and then store the info in the `struct libevdev``
    libevdev_new_from_fd(fd, &dev);
    printf("Device: %s\n", libevdev_get_name(dev));
    printf("Listening for key events...\n");
    fds[i] = fd;
    devs[i] = dev;
  }

  // struct input_event, any kind of event that the device emits.
  // This event is what libevdev generalises for us. Making it
  // "type safe"
  struct input_event ev;
  fd_set fdSet;
  int maxFd = 0;
  while (1) {
    // Initialization step as per manual page
    // Clears all the fds from the set
    FD_ZERO(&fdSet);

    for (int i = 0; i < set->size; i++) {
      // it adds a fd(here fds[i]) to the set (fdSet)
      FD_SET(fds[i], &fdSet);
      if (fds[i] > maxFd)
        maxFd = fds[i];
    }

    // This is where the proc will suspend itself and then will
    // wake up if, either any one of the fd is ready or the
    // timeout expires.
    //
    // In this case, we do not set any Timeout, -> timeout param
    // is passed NULL. => Indefinite suspension. Will only resume
    // if an event occurs.
    //
    // The set (fdSet) is modified by the select internally. Upon
    // returning, the set only contains those fds that are ready
    // and thus we need to initialize the set every time we have
    // to `select` from the given fds.
    int ready = select(maxFd + 1, &fdSet, NULL, NULL, NULL);
    if (ready < 0) {
      perror("select");
      exit(1);
    }

    for (int i = 0; i < set->size; i++) {
      // if the current fd is not ready, i.e. not in set by select
      // we continue.
      //
      // this checks if the mentioned fd is in the set after
      // `select` has performed its ops
      if (!FD_ISSET(fds[i], &fdSet))
        continue;

      // This is neither busy waiting nor it suspends the proc
      // until it recieves any event. It just checks if there
      // has been an event since last checked. Here, we exec it
      // in normal mode(?)
      //    -> returns the LIBEVDEV_READ_STATUS_SUCCESS
      //       if event occurred
      //       else
      //       returns -EAGAIN and sets ev to NULL
      if (libevdev_next_event(devs[i], LIBEVDEV_READ_FLAG_NORMAL, &ev) != 0)
        continue;

      if (ev.type == EV_KEY && ev.value == 1) { // Key press event
        printf("Key: %s\n", libevdev_event_code_get_name(ev.type, ev.code));
        if (ev.code == KEY_N && (ev.code & KEY_LEFTCTRL)) {
          printf("CTRL + N detected!\n");
        }
      }
    }
  }

  for (int i = 0; i < set->size; i++) {
    libevdev_free(devs[i]);
    close(fds[i]);
  }
  return 0;
}

// Src:
// https://github.com/horrifyingHorse/Segmentation-Fault-Dump/commit/2b4c1a7d5382a36ad05a576037adf0928037405c
int getKbdEvents(struct IntSet **set) {
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

  *set = NULL;
  initSet(set, 1);
  struct dirent *entry = NULL;

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
    pushSet(*set, atoi((symlinkTo + strlen(symlinkTo) - 1)));
  }

  closedir(dir);

  int size = (*set)->size;
  printf("eventX is a keyboard Event | X =  ");
  while (size-- > 0) {
    printf("%d, ", (*set)->set[size]);
  }
  printf("\b\b;\n");

  return 0;
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

int intLen(int n) {
  int count = 1;
  while ((n /= 10) != 0) {
    count++;
  }
  return count;
}
