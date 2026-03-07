#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parsePGM.c"
#include "producer.c"

// consumer

void* consumer_thread(void* arg) {
  (void)arg;

  while (1) {
    Block item;
    int local_hist[HIST_SIZE];
    int i;

    if (!buffer_get(&cb, &item)) {
      break;
    }

    for (i = 0; i < HIST_SIZE; i++) {  // build local histogram for each
                                       // consumer, at the end all merge
      local_hist[i] = 0;
    }

    for (i = 0; i < item.nbytes; i++) {
      local_hist[item.data[i]]++;
    }

    pthread_mutex_lock(&hist_lock);  // prevent race conditions
    for (i = 0; i < HIST_SIZE; i++) {
      histogram[i] += local_hist[i];
    }
    pthread_mutex_unlock(&hist_lock);

    free(item.data);
  }

  pthread_exit(NULL);
}
