#ifndef CONSUMER_H
#define CONSUMER_H
#include "producer.h"

void* consumer_thread(void* arg);
void buffer_init(CircularBuffer* b, int capacity);
void buffer_destroy(CircularBuffer* b);
void buffer_put(CircularBuffer* b, unsigned char* data, int nbytes);
int buffer_get(CircularBuffer* b, Block* item);
#endif