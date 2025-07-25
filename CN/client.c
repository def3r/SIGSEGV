#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void error(const char *msg);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sockfd, portno, n;
  char bffr[256];

  struct hostent *server;
  struct sockaddr_in servAddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("Error opening socket");
  }

  portno = atoi(argv[2]);
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    error("No such host.");
  }

  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr,
        server->h_length);
  servAddr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    error("Connection Failed.");
  }

  while (1) {
    bzero(bffr, 256);
    fgets(bffr, 256, stdin);

    n = write(sockfd, bffr, strlen(bffr));
    if (n < 0) {
      error("Error on writing.");
    }

    bzero(bffr, 256);
    n = read(sockfd, bffr, 256);
    if (n < 0) {
      error("Error on reading.");
    }
    printf("Server: %s", bffr);

    if (!strncmp("exit", bffr, 4)) {
      break;
    }
  }
  close(sockfd);
  return EXIT_SUCCESS;
}

void error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
