#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 3001
#define BACKLOG 9

void error(const char *msg);
void check(int *var, const char *msg);
char *calcRes(char *bfr);

long sum(long operands[2]) { return operands[0] + operands[1]; }
long diff(long operands[2]) { return operands[0] - operands[1]; }
long mult(long operands[2]) { return operands[0] * operands[1]; }
long divi(long operands[2]) { return operands[0] / operands[1]; }
long mod(long operands[2]) { return operands[0] % operands[1]; }

// NOTE: Flexible Array Member in a struct{}
#define MAPSIZE 5
struct map {
  long (*opFunc[MAPSIZE])(long[2]);
  char *op[MAPSIZE];
};
struct map *initMap() {
  struct map *m = malloc(sizeof(struct map));
  m->op[0] = "+";
  m->opFunc[0] = sum;
  m->op[1] = "-";
  m->opFunc[1] = diff;
  m->op[2] = "*";
  m->opFunc[2] = mult;
  m->op[3] = "/";
  m->opFunc[3] = divi;
  m->op[4] = "%";
  m->opFunc[4] = mod;
  return m;
}

struct map *m;

int main() {
  int sockfd, clientfd;
  struct sockaddr_in servAddr, clientAddr;
  socklen_t clientLen;
  size_t bytes;
  m = initMap();

  char bfr[BUFSIZ];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  check(&sockfd, "Cannot create a socket.");

  // bzero sucks, memset rocks
  // https://stackoverflow.com/questions/17096990/why-use-bzero-over-memset
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    error("Binding Failed");
  };
  listen(sockfd, BACKLOG);

  clientLen = sizeof(clientAddr);
  clientfd = accept(sockfd, (struct sockaddr *)&servAddr, &clientLen);
  check(&clientfd, "Cannot accept the connection");

  strcpy(bfr, "Format: int int op\nSupported Ops: + - / %");
  while (1) {
    bytes = write(clientfd, bfr, strlen(bfr));
    check((int *)&bytes, "Cannot write to client");

    bytes = read(clientfd, bfr, BUFSIZ);
    check((int *)&bytes, "Cannot read from client");
    printf("rec: %s\n", bfr);
    calcRes(bfr);
    printf("Calc: %s\n", bfr);
    fflush(stdout);
  }

  close(clientfd);
  close(sockfd);
  return EXIT_SUCCESS;
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

char *calcRes(char *bfr) {
  long operands[2];
  int operandNo = 0;
  char op[BUFSIZ];
  char *it = bfr, *start = bfr;

  while (*it != '\0' && operandNo < 2) {
    if (*it == ' ') {
      *it = '\0';
      operands[operandNo++] = atoi(start);
      start = it + 1;
      printf("->%s\n", start);
    }
    it++;
  }
  strcpy(op, start);
  memset(bfr, 0, BUFSIZ);

  long res = 0;
  for (int i = 0; i < MAPSIZE; i++) {
    printf("|%s| |%s|\n", op, m->op[i]);
    if (strncmp(m->op[i], op, 1) == 0) {
      res = m->opFunc[i](operands);
      break;
    }
  }
  sprintf(bfr, "%ld", res);

  return NULL;
}
