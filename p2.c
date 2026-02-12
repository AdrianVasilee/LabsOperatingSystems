#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "circularBuffer.h"
#include "splitCommand.h"

#define MAX_LINE 1024
#define READ_SIZE 128
int get_line_from_buffer(CircularBuffer* cb, char* dest, int size) {
  int reachedEOF = 0;
  int line_size = buffer_size_next_element(cb, '\n', reachedEOF);  // like p1
  if (line_size <= 0) return -1;
  int i;
  for (i = 0; i < line_size && i < size - 1; i++) {  // like p1
    dest[i] = buffer_pop(cb);
  }
  dest[i] = '\0';
  return i;
}
int fill_buffer(CircularBuffer* cb) {
  unsigned char temp[READ_SIZE];
  int bytes_read = read(STDIN_FILENO, temp, READ_SIZE);
  if (bytes_read <= 0) return bytes_read;
  for (int i = 0; i < bytes_read; i++) {  // if enough space fill it
    if (buffer_free_bytes(cb) > 0) {
      buffer_push(cb, temp[i]);
    }
  }
  return bytes_read;
}
void wait_for_line(CircularBuffer* cb,
                   char* dest) {  // basically while reading the buffer
  while (get_line_from_buffer(cb, dest, MAX_LINE) == -1) {
    if (fill_buffer(cb) == 0) exit(0);
  }
}

void run_piped(char* line1, char* line2) {
  int fd[2];
  if (pipe(fd) == -1) return;

  pid_t pid1 = fork();
  if (pid1 == 0) {
    dup2(fd[1], STDOUT_FILENO);
    close(fd[0]);
    close(fd[1]);
    char** argv1 = split_command(line1);
    execvp(argv1[0], argv1);
    exit(1);
  }

  pid_t pid2 = fork();
  if (pid2 == 0) {
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    close(fd[1]);
    char** argv2 = split_command(line2);
    execvp(argv2[0], argv2);
    exit(1);
  }
  close(fd[0]);
  close(fd[1]);
  waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);
}

int main() {
  CircularBuffer cb;
  buffer_init(&cb, 2048);

  char mode[MAX_LINE];
  char cmd1[MAX_LINE];
  char cmd2[MAX_LINE];

  while (1) {
    wait_for_line(&cb, mode);
    mode[strcspn(mode, "\n")] = 0;

    if (strcmp(mode, "EXIT") == 0) {  // compares
      break;
    } else if (strcmp(mode, "SINGLE") == 0) {
      // TODO
    } else if (strcmp(mode, "CONCURRENT") == 0) {
      // TODO
    } else if (strcmp(mode, "PIPE") == 0) {
      wait_for_line(&cb, cmd1);
      wait_for_line(&cb, cmd2);
      run_piped(cmd1, cmd2);
    }
  }

  buffer_deallocate(&cb);
  return 0;
}