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
  if (argc < 2) {
    fprintf(stderr, "Port not provided\n");
    exit(EXIT_FAILURE);
  }

  int sockfd, newsockfd, portno, n;
  char bffr[256];

  struct sockaddr_in servAddr, cliAddr;
  socklen_t cliLen;

  // Address family
  // AF_INET -> IPv4
  // AF_INET6 -> IPv6
  //
  // Protocol Type
  // SOCK_STREAM -> TCP
  // SOCK_DGRAM -> UDP
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // socket(...) creates a socket in the namespace (address family) but has no
  // address assigned to it. Thus, we need to bind(...) the socket to the addr

  if (sockfd < 0) {
    error("Error opening socket");
  }

  // bzero sucks, memset rocks
  // https://stackoverflow.com/questions/17096990/why-use-bzero-over-memset
  bzero((char *)&servAddr, sizeof(servAddr));
  portno = atoi(argv[1]);

  servAddr.sin_family = AF_INET;
  // socket addr when set to INADDR_ANY (0), it accepts connection to all the
  // IPs of the machine (any interface)
  // INADDR_LOOPBACK --> localhost
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(portno);
  // htons(host to network, short) makes sure that numbers are stored in memory
  // in network byte order
  // Network Byte Order is different than big-endian,
  // lil-endian & Network Byte Order depending on machine and network protocol
  // in use.
  // https: //
  // stackoverflow.com/questions/19207745/htons-function-in-socket-programing

  // bind(...) assigns addr pointed by sockaddr*
  // to socket reffered by the socket fd
  //
  // This op is called "assigning a name to a socket"
  //
  // Rules in name binding vary between address families
  if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    error("Binding Failed.");
  }

  // listens to fd, backlog is the maximum number of pending connections the
  // kernel should queue for the socket.
  // Acc to man:
  // listen() marks the socket referred to by the sockfd as a passive socket,
  // that is, as a socket that will be used to accept incoming connection
  // requests using accept.2
  listen(sockfd, 9);
  cliLen = sizeof(cliAddr);

  // stores the accepted client addr in cliAddr
  // accept need -> struct sockaddr*
  // cliAddr     -> struct sockaddr_in
  // polymorohism.
  // sockaddr is a generic descriptor for anu kind of socket op.
  // sockaddr_in is a struct specific for IP-based comms ("in" stands for
  // "InterNet")
  // https://stackoverflow.com/questions/21099041/why-do-we-cast-sockaddr-in-to-sockaddr-when-calling-bind
  newsockfd = accept(sockfd, (struct sockaddr *)&cliAddr, &cliLen);
  // sockfd is the listening socket, and accept returns the connected client fd,
  // Acc to man:
  // It extracts the first connection request on the queue of pending
  // connections for the listening socket, sockfd, creates a new connector
  // socket, and returns a new file descriptor referring to that socket. The
  // newly created socket is not in listening state. The original socket, sockfd
  // is unaffected by this call.

  if (newsockfd < 0) {
    error("Error on Accept.");
  }

  while (1) {
    bzero(bffr, 256);
    n = read(newsockfd, bffr, 256);
    if (n < 0) {
      error("Error on reading.");
    }
    printf("Client: %s\n", bffr);
    bzero(bffr, 256);
    fgets(bffr, 256, stdin);

    n = write(newsockfd, bffr, strlen(bffr));
    if (n < 0) {
      error("Error on writing.");
    }

    if (!strncmp("exit", bffr, 4)) {
      break;
    }
  }

  close(newsockfd);
  close(sockfd);
  return 0;
}

void error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
