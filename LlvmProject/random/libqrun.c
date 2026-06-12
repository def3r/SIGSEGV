#include "libqrun.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ── embedded worker via linker trick ─────────────────────────────────────────
// ld -r -b binary qworker.py -o qworker.o
extern char _binary_qworker_py_start[];
extern char _binary_qworker_py_end[];

// ── worker state ─────────────────────────────────────────────────────────────
static FILE* worker_in  = NULL;
static FILE* worker_out = NULL;
static pid_t worker_pid = -1;
static char  worker_tmp[64] = "/tmp/qworker_XXXXXX.py";

// ── protocol helpers ─────────────────────────────────────────────────────────

static void send_null_terminated(const char* s, size_t len) {
  fwrite(s, 1, len, worker_in);
  fputc('\0', worker_in);
  fflush(worker_in);
}

static char* read_until_null() {
  size_t cap = 256;
  size_t len = 0;
  char*  buf = malloc(cap);

  if (!buf) {
    fprintf(stderr, "quantum_runtime: malloc failed\n");
    abort();
  }

  int ch;
  while ((ch = fgetc(worker_out)) != '\0') {
    if (ch == EOF) {
      fprintf(stderr, "quantum_runtime: worker pipe closed unexpectedly\n");
      free(buf);
      abort();
    }
    if (len + 1 >= cap) {
      cap *= 2;
      buf  = realloc(buf, cap);
      if (!buf) {
        fprintf(stderr, "quantum_runtime: realloc failed\n");
        abort();
      }
    }
    buf[len++] = (char)ch;
  }
  buf[len] = '\0';
  return buf;
}

// ── table lookup ─────────────────────────────────────────────────────────────

static quantum_entry_t* quantum_lookup(const char* key) {
  for (int i = 0; i < quantum_table_size; i++) {
    if (strcmp(quantum_table[i].key, key) == 0) {
      return &quantum_table[i];
    }
  }
  return NULL;
}

// ── constructor: init worker before main ─────────────────────────────────────

__attribute__((constructor)) static void quantum_runtime_init() {
  int fd = mkstemps(worker_tmp, 3);
  if (fd < 0) {
    fprintf(stderr, "quantum_runtime: failed to create worker tmp file\n");
    abort();
  }

  // use linker symbols instead of xxd array
  size_t worker_len = _binary_qworker_py_end - _binary_qworker_py_start;
  write(fd, _binary_qworker_py_start, worker_len);
  close(fd);

  int stdin_pipe[2], stdout_pipe[2];
  if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
    fprintf(stderr, "quantum_runtime: pipe() failed\n");
    abort();
  }

  worker_pid = fork();
  if (worker_pid < 0) {
    fprintf(stderr, "quantum_runtime: fork() failed\n");
    abort();
  }

  if (worker_pid == 0) {
    dup2(stdin_pipe[0],  STDIN_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    execlp("python3", "python3", worker_tmp, NULL);
    fprintf(stderr, "quantum_runtime: execlp failed\n");
    exit(1);
  }

  close(stdin_pipe[0]);
  close(stdout_pipe[1]);
  worker_in  = fdopen(stdin_pipe[1], "w");
  worker_out = fdopen(stdout_pipe[0], "r");

  if (!worker_in || !worker_out) {
    fprintf(stderr, "quantum_runtime: fdopen failed\n");
    abort();
  }

  char* ready = read_until_null();
  if (strcmp(ready, "READY") != 0) {
    fprintf(stderr, "quantum_runtime: worker did not send READY, got: %s\n", ready);
    free(ready);
    abort();
  }
  free(ready);

  fprintf(stderr, "quantum_runtime: worker ready\n");
}

// ── destructor ───────────────────────────────────────────────────────────────

__attribute__((destructor)) static void quantum_runtime_destroy() {
  if (worker_in)  fclose(worker_in);
  if (worker_out) fclose(worker_out);
  if (worker_pid > 0) kill(worker_pid, SIGTERM);
  if (worker_tmp[0])  unlink(worker_tmp);
}

// ── quantum_execute ───────────────────────────────────────────────────────────

int32_t quantum_execute(const char* key, const char* decoder) {
  quantum_entry_t* entry = quantum_lookup(key);
  if (!entry) {
    fprintf(stderr, "quantum_runtime: no table entry for key: %s\n", key);
    abort();
  }

  send_null_terminated(decoder, strlen(decoder));
  send_null_terminated(entry->qasm, entry->qasm_len);

  char* status = read_until_null();
  if (strcmp(status, "ERR") == 0) {
    free(status);
    char* msg = read_until_null();
    fprintf(stderr, "quantum_runtime: worker error: %s\n", msg);
    free(msg);
    abort();
  }
  free(status);

  char*   result_str = read_until_null();
  int32_t result     = (int32_t)atoi(result_str);
  free(result_str);

  return result;
}
