#include "producer.h"
#include "buffer.h"

int producer;

int blockSize = 1024 * 16;
// Position from which the Producer will try to read a block. This needs to be initialised to just after the header.
int readPos; 
pthread_mutex_t lock_read;

void * Producer (void* arg) {
    char * path = (char *) arg; // Need to pass the file as a path
    int fd = open(path, O_RDONLY);
    int bytesRead;
    int readPosLocal;

    while (1) {
        pthread_mutex_lock(&lock_read);
        readPosLocal = readPos;
        readPos += blockSize;
        pthread_mutex_unlock(&lock_read);

        // Produce an item by reading a block
        lseek(fd, readPosLocal, SEEK_SET);
        unsigned char* buff = malloc(blockSize);
        bytesRead = read(fd, buff, blockSize);
        if (bytesRead <= 0) {
            free(buff);
            break;
        }
        
        // Consumer part of adding to the buffer: need to 
        pthread_mutex_lock(&lockBuffer);
        while(nBuffer == elementsInBuffer){
            pthread_cond_wait(&emptyBuffer, &lockBuffer);
        }

        Buffer[elementsInBuffer] = buff;
        nrBytesRead[elementsInBuffer] = bytesRead; 
        elementsInBuffer++;
        
        pthread_mutex_unlock(&lockBuffer);

        pthread_cond_signal(&fullBuffer);
    }

    pthread_cond_signal(&fullBuffer);
    producersFinished = 1;
    // If exiting, make sure you wake up all sleeping threads before exiting 
    // (and that they don't go to sleep if the finishes)
    return 0;
}
