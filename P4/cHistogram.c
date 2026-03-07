#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "consumer.h"
#include "parsePGM.h"
#include "producer.h"

// output

int write_histogram(const char* path) {  // write final histogram in a txt file
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {  // if open fails, error
    return -1;
  }

  char line[64];
  int i;
  for (i = 0; i < HIST_SIZE; i++) {
    int len = sprintf(line, "%d %d\n", i,
                      histogram[i]);  // prints for each pixel color, number of
                                      // pixels of such color
    if (write(fd, line, len) != len) {
      close(fd);
      return -1;
    }
  }

  close(fd);
  return 0;
}

// main

int main(int argc, char** argv) {
  if (argc != 6) {
    write(
        2,
        "Usage: computeHistogram input.pgm output.txt Nprod Ncons sizeBuffer\n",
        68);
    return 1;
  }

  input_path = argv[1];
  const char* output_path = argv[2];
  int nprod = atoi(argv[3]);
  int ncons = atoi(argv[4]);
  int sizeBuffer = atoi(argv[5]);

  if (nprod <= 0 || ncons <= 0 || sizeBuffer <= 0) {
    write(2, "Invalid arguments\n", 18);
    return 1;
  }

  int width, height, maxval;
  int headerSize = parse_pgm_header(input_path, &width, &height, &maxval);
  if (headerSize < 0) {
    write(2, "Invalid PGM file\n", 17);
    return 1;
  }

  readPos = headerSize;
  endPos = headerSize + width * height;
  active_producers = nprod;

  int i;
  for (i = 0; i < HIST_SIZE; i++) {
    histogram[i] = 0;
  }

  buffer_init(&cb, sizeBuffer);

  pthread_t* producers = (pthread_t*)malloc(sizeof(pthread_t) * nprod);
  pthread_t* consumers = (pthread_t*)malloc(sizeof(pthread_t) * ncons);

  if (producers == NULL || consumers == NULL) {
    write(2, "malloc error\n", 13);
    free(producers);
    free(consumers);
    buffer_destroy(&cb);
    return 1;
  }

  for (i = 0; i < nprod; i++) {
    pthread_create(&producers[i], NULL, producer_thread, NULL);
  }

  for (i = 0; i < ncons; i++) {
    pthread_create(&consumers[i], NULL, consumer_thread, NULL);
  }

  for (i = 0; i < nprod; i++) {
    pthread_join(producers[i], NULL);
  }

  for (i = 0; i < ncons; i++) {
    pthread_join(consumers[i], NULL);
  }

  if (write_histogram(output_path) < 0) {
    write(2, "output error\n", 13);
  }

  free(producers);
  free(consumers);
  buffer_destroy(&cb);

  return 0;
}