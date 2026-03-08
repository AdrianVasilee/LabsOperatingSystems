#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>

unsigned char **Buffer;
int *nrBytesRead;
int nBuffer;
int elementsInBuffer;
int maxval;

int producersFinished = 0;

pthread_mutex_t lockBuffer;
pthread_cond_t emptyBuffer;
pthread_cond_t fullBuffer;
pthread_cond_t finished;

#endif