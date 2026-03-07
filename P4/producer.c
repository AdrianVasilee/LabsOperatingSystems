#include "producer.h"

#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "consumer.h"
#include "parsePGM.h"
#define BLOCK_SIZE (1024 * 16)
#define HIST_SIZE 256

// what each buffer slot stores
// typedef struct {
//   unsigned char* data;
//   int nbytes;
// } Block;

// typedef struct {
//   Block* buffer;
//   int capacity;
//   int in;                // where next producer puts info
//   int out;               // where consumer removes data
//   int count;             // how many blocks are being used
//   pthread_mutex_t lock;  // protect shared buffer state
//   pthread_cond_t full;   // producers wait here
//   pthread_cond_t empty;  // consumers wait here
// } CircularBuffer;

// // global shared state

// static CircularBuffer cb;

// static int histogram[HIST_SIZE];
// static pthread_mutex_t hist_lock =
//     PTHREAD_MUTEX_INITIALIZER;  // protects updates so there's no race
//                                 // conditions if two+ consumers try to add
//                                 info
//                                 // to histogram at the same time

// // shared read position for when multiple producers
// static int readPos;
// static int endPos;
// static pthread_mutex_t read_lock =
//     PTHREAD_MUTEX_INITIALIZER;  // solve race condition for when 2 producers
//                                 // could read same readPos before
//                                 incrementing

// // producers still running
// static int active_producers;
// static pthread_mutex_t prod_lock = PTHREAD_MUTEX_INITIALIZER;

// static const char* input_path;

void* producer_thread(void* arg) {
  (void)arg;

  int fd =
      open(input_path, O_RDONLY);  // each producer opens the file independently
  if (fd < 0) {  // if open fails, a producer dies so we need to wake the
                 // consumer so its not waiting forever
    pthread_mutex_lock(&prod_lock);
    active_producers--;
    int last = (active_producers == 0);
    pthread_mutex_unlock(&prod_lock);

    if (last) {
      pthread_mutex_lock(&cb.lock);
      pthread_cond_broadcast(&cb.empty);
      pthread_mutex_unlock(&cb.lock);
    }

    pthread_exit(NULL);
  }

  while (1) {
    int myPos;

    pthread_mutex_lock(&read_lock);
    myPos = readPos;
    readPos += BLOCK_SIZE;
    pthread_mutex_unlock(&read_lock);

    if (myPos >= endPos) {  // stop at EOF
      break;
    }

    int toRead = BLOCK_SIZE;  // makes last part shorter if needed in case
                              // there's not 16,834 bytes
    if (myPos + toRead > endPos) {
      toRead = endPos - myPos;
    }

    unsigned char* buff = (unsigned char*)malloc(
        toRead);  // eacg produced item gets own allocated block
    if (buff == NULL) {
      break;
    }

    lseek(fd, myPos, SEEK_SET);
    int nBytesRead = read(fd, buff, toRead);
    // if (nBytesRead <= 0) {
    //   free(buff);
    //   break;
    // }

    buffer_put(&cb, buff, nBytesRead);
  }

  close(fd);

  // update 'there's still producers'
  pthread_mutex_lock(&prod_lock);
  active_producers--;
  int last = (active_producers == 0);
  pthread_mutex_unlock(&prod_lock);

  // wake all sleeping consumers when the last producer finishes
  if (last) {
    pthread_mutex_lock(&cb.lock);
    pthread_cond_broadcast(&cb.empty);
    pthread_mutex_unlock(&cb.lock);
  }

  pthread_exit(NULL);
}
