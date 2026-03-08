#include "consumer.h"
#include "buffer.h"

void * Consumer (void *) {
    unsigned char *buff;
    int bytesRead;

    unsigned int* histogram = malloc(maxval  * sizeof(unsigned int));
    for (int i = 0; i < maxval; i++) {
        histogram[i] = 0;
    }

    while (1) {
        pthread_mutex_lock(&lockBuffer);
        while(elementsInBuffer == 0){
            if (producersFinished) break;
            pthread_cond_wait(&fullBuffer, &lockBuffer);
        }
        
        if (producersFinished && elementsInBuffer == 0) {
            pthread_mutex_unlock(&lockBuffer);
            break;
        }

        elementsInBuffer--;
        buff = Buffer[elementsInBuffer];
        bytesRead = nrBytesRead[elementsInBuffer];
        
        nrBytesRead[elementsInBuffer] = 0;
        
        pthread_mutex_unlock(&lockBuffer);

        pthread_cond_signal(&emptyBuffer);

        for (int i = 0; i < bytesRead; i++) {
            histogram[buff[i]]++;
        }

        free(buff);
    }

    return (void *) histogram;
}