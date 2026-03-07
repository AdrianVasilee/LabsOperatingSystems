#include "consumer.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parsePGM.h"
#include "producer.h"

// consumer

void buffer_init(CircularBuffer* b, int capacity) {
  b->buffer = (Block*)malloc(sizeof(Block) * capacity);
  b->capacity = capacity;
  b->in = 0;
  b->out = 0;
  b->count = 0;
  pthread_mutex_init(&b->lock, NULL);
  pthread_cond_init(&b->full, NULL);
  pthread_cond_init(&b->empty, NULL);
}

void buffer_destroy(CircularBuffer* b) {
  free(b->buffer);
  pthread_mutex_destroy(&b->lock);
  pthread_cond_destroy(&b->full);
  pthread_cond_destroy(&b->empty);
}

void buffer_put(CircularBuffer* b, unsigned char* data, int nbytes) {
  pthread_mutex_lock(&b->lock);  // lock when putting so 2 producers dont write
                                 // at the same time

  while (b->count == b->capacity) {  // fixes busy-wait
    pthread_cond_wait(&b->full, &b->lock);
  }

  // actual put of the buffer
  b->buffer[b->in].data = data;
  b->buffer[b->in].nbytes = nbytes;
  b->in = (b->in + 1) % b->capacity;
  b->count++;

  pthread_cond_signal(&b->empty);
  pthread_mutex_unlock(&b->lock);
}

int buffer_get(CircularBuffer* b, Block* item) {
  pthread_mutex_lock(&b->lock);  // prevent races again

  while (b->count == 0) {  // when the buffer is empty and all producers
                           // finished, consumers stop
    pthread_mutex_lock(&prod_lock);
    int finished = (active_producers == 0);
    pthread_mutex_unlock(&prod_lock);

    if (finished) {
      pthread_mutex_unlock(&b->lock);
      return 0;  // finish
    }

    pthread_cond_wait(&b->empty, &b->lock);
  }

  *item = b->buffer[b->out];  // remove item
  b->out = (b->out + 1) % b->capacity;
  b->count--;

  pthread_cond_signal(&b->full);
  pthread_mutex_unlock(&b->lock);
  return 1;
}
void* consumer_thread(void* arg) {
  (void)arg;

  while (1) {
    Block item;
    int local_hist[HIST_SIZE];
    int i;

    // Initialize local_hist BEFORE buffer_get
    for (i = 0; i < HIST_SIZE; i++) {
      local_hist[i] = 0;
    }

    if (!buffer_get(&cb, &item)) {
      break;
    }

    // Now process the data
    for (i = 0; i < item.nbytes; i++) {
      local_hist[item.data[i]]++;
    }

    pthread_mutex_lock(&hist_lock);
    for (i = 0; i < HIST_SIZE; i++) {
      histogram[i] += local_hist[i];
    }
    pthread_mutex_unlock(&hist_lock);

    free(item.data);
  }

  pthread_exit(NULL);
}