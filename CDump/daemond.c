#include <stdio.h>
#include <unistd.h>

// dihmon
//
// SysV Daemon:
// A daemon should not have its controlling terminal
// There exists a func. daemon(int nochdir, int noclose);
// in unistd.h
// Altho ther is a known bug for it where:
//    "if the daemon opens a terminal that is not already
//     a controlling terminal for another session, then
//     that terminal will inadvertently become the
//     controlling terminal for the daemon."
// src: https://lloydrochester.com/post/c/unix-daemon-example/
//
// Manual way of creating a daemon is a double fork method.
//
// systemdih:
// systemd handles everything.
//
// Unit file -> *.service
// Need not be a symlink, but why not
// $ sudo ln -s $(pwn)/daemond.service /etc/systemd/system/
// $ sudo systemctl daemon-reload
// $ sudo systemctl enable daemond.service
// $ sudo systemctl start daemond.service
// $ sudo systemctl status daemond.service
//
// For the logs (both stderr and stdout):
// $ journalctl -u daemond.service -f
// shows the most recent(-f) logs for unit daemond.service
// Can also use -o to output the logs to json(see man journalctl)

int main() {
  printf("Oh no! systemdih\n");
  return 0;
}
