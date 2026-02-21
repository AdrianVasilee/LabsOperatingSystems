#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parsePGM.h"

#define BUFF_SIZE 1024

typedef struct {
  char* path;
  int offset;  // starting position in the file
  int bytesToRead;
  unsigned int* globalHistogram;
  pthread_mutex_t* mutex;
  int maxval;
} ThreadInfo;

void* compute_local_histogram(void* arg) {
  ThreadInfo* info = (ThreadInfo*)arg;
  int fd = open(info->path, O_RDONLY);
  if (fd < 0) {
    perror("Thread failed to open file");
    return NULL;
  }
  lseek(fd, info->offset, SEEK_SET);
  unsigned int* localHist = calloc(info->maxval + 1, sizeof(unsigned int));
  unsigned char buffer[BUFF_SIZE];

  int totalRead = 0;
  while (totalRead < info->bytesToRead) {
    int toRead = (info->bytesToRead - totalRead > BUFF_SIZE)
                     ? BUFF_SIZE
                     : (info->bytesToRead - totalRead);

    int nBytes = read(fd, buffer, toRead);
    if (nBytes <= 0) break;

    for (int i = 0; i < nBytes; i++) {
      localHist[buffer[i]]++;
    }
    totalRead += nBytes;
  }

  pthread_mutex_lock(info->mutex);
  for (int i = 0; i <= info->maxval; i++) {
    info->globalHistogram[i] += localHist[i];
  }
  pthread_mutex_unlock(info->mutex);
  free(localHist);
  close(fd);
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Usage: %s <pathToImage> <pathToHistogramOut> <numThreads>\n",
           argv[0]);
    exit(1);
  }

  char* imagePath = argv[1];
  char* outputPath = argv[2];
  int numThreads = atoi(argv[3]);
  int width, height, maxval;
  int nBytesHeader = parse_pgm_header(imagePath, &width, &height, &maxval);

  if (nBytesHeader < 0) {
    fprintf(stderr, "Error parsing PGM header\n");
    exit(1);
  }
  if (maxval > 255) {
    fprintf(stderr, "Expecting 1 byte pixels (maxval <= 255)\n");
    exit(1);
  }

  long nPixels = (long)width * height;
  unsigned int* globalHistogram = calloc(maxval + 1, sizeof(unsigned int));
  pthread_mutex_t histMutex;
  pthread_mutex_init(&histMutex, NULL);
  pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
  ThreadInfo* threadInfos = malloc(numThreads * sizeof(ThreadInfo));
  int pixelsPerThread = nPixels / numThreads;
  int remainingPixels = nPixels % numThreads;

  int currentOffset = nBytesHeader;

  for (int i = 0; i < numThreads; i++) {
    threadInfos[i].path = imagePath;
    threadInfos[i].offset = currentOffset;
    threadInfos[i].globalHistogram = globalHistogram;
    threadInfos[i].mutex = &histMutex;
    threadInfos[i].maxval = maxval;

    // Distribute the remainder to the last thread
    if (i == numThreads - 1) {
      threadInfos[i].bytesToRead = pixelsPerThread + remainingPixels;
    } else {
      threadInfos[i].bytesToRead = pixelsPerThread;
    }

    pthread_create(&threads[i], NULL, compute_local_histogram, &threadInfos[i]);

    currentOffset += threadInfos[i].bytesToRead;
  }
  for (int i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }
  int fd_out = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd_out < 0) {
    perror("Failed to open output file");
  } else {
    for (int i = 0; i <= maxval; i++) {
      char s[80];
      int len = sprintf(s, "%d,%u\n", i, globalHistogram[i]);
      write(fd_out, s, len);
    }
    close(fd_out);
  }
  pthread_mutex_destroy(&histMutex);
  free(globalHistogram);
  free(threads);
  free(threadInfos);

  return 0;
}