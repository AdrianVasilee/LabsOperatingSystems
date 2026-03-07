
#ifndef PRODUCER_H
#define PRODUCER_H

#include <pthread.h>

#define BLOCK_SIZE (1024 * 16)
#define HIST_SIZE 256

typedef struct {
  unsigned char* data;
  int nbytes;
} Block;

typedef struct {
  Block* buffer;
  int capacity;
  int in;
  int out;
  int count;
  pthread_mutex_t lock;
  pthread_cond_t full;
  pthread_cond_t empty;
} CircularBuffer;
static CircularBuffer cb;

static int histogram[HIST_SIZE];
static pthread_mutex_t hist_lock =
    PTHREAD_MUTEX_INITIALIZER;  // protects updates so there's no race
                                // conditions if two+ consumers try to add info
                                // to histogram at the same time

// shared read position for when multiple producers
static int readPos;
static int endPos;
static pthread_mutex_t read_lock =
    PTHREAD_MUTEX_INITIALIZER;  // solve race condition for when 2 producers
                                // could read same readPos before incrementing

// producers still running
static int active_producers;
static pthread_mutex_t prod_lock = PTHREAD_MUTEX_INITIALIZER;

static const char* input_path;
void* producer_thread(void* arg);
#endif