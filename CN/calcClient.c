#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 3001
#define HOSTNAME "localhost"

void error(const char *msg);
void check(int *var, const char *msg);

int main() {
  int sockfd;
  struct hostent *server;
  struct sockaddr_in servAddr;
  size_t bytes;
  char bfr[BUFSIZ];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  check(&sockfd, "Cannot create a socket.");

  server = gethostbyname(HOSTNAME);

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  memcpy(&servAddr.sin_addr.s_addr, server->h_addr, server->h_length);
  servAddr.sin_port = htons(PORT);

  if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    error("Connection failed");
  }

  while (1) {
    memset(bfr, 0, BUFSIZ);
    bytes = read(sockfd, bfr, BUFSIZ);
    check((int *)&bytes, "Cannot read from server");
    printf("%s\n", bfr);

    memset(bfr, 0, BUFSIZ);
    fgets(bfr, BUFSIZ, stdin);
    bytes = write(sockfd, bfr, sizeof(bfr));
    check((int*)&bytes, "Cannot write to server");
  }

  return 0;
}

void error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void check(int *var, const char *msg) {
  if (var == NULL) {
    error(msg);
  }
  if (*var < 0) {
    error(msg);
  }
}
