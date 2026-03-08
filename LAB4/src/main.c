#include "producer.c"
#include "parsePGM.c"
#include "buffer.h"
#include "consumer.c"
#include <unistd.h>
#include <pthread.h>

int main(int argc, char *argv[]) {

     if (argc != 6) {
        printf("Incorrect usage. Args needed: pathToImage pathToHistogramOut numberOfProducers numberOfConsumers sizeBuffer \n");
        _exit(1);
    }

    int nrProducers = atoi(argv[3]);
    int nrConsumers = atoi(argv[4]);
    nBuffer = atoi(argv[5]);

    // Read the header
    int width, height;
    int nBytesHeader = parse_pgm_header(argv[1], &width, &height, &maxval);

    maxval ++;

    if (maxval > 256){
        perror("Expecting 1 byte ints\n");
        _exit(1);
    }

    Buffer = malloc(sizeof(unsigned char**) * nBuffer);
    nrBytesRead = malloc(sizeof(int) * nBuffer);

    readPos = nBytesHeader;

    // start producers
    pthread_t *producer_threads = malloc(sizeof(pthread_t) * nrProducers);

    for (int i = 0; i < nrProducers; i++) {
        pthread_create(&producer_threads[i], NULL, Producer, (void *) argv[1]);
    }

    // start consumers
    pthread_t *consumer_threads = malloc(sizeof(pthread_t) * nrConsumers);
    for (int i = 0; i < nrConsumers; i++) {
        pthread_create(&consumer_threads[i], NULL, Consumer, NULL);
    }

    // join producers
    for (int i = 0; i < nrProducers; i++) {
        pthread_join(producer_threads[i], NULL);
    }

    // generate total histogram
    unsigned int * totalHistogram = malloc(maxval * sizeof(unsigned int));

    for (int i = 0; i < maxval; i++)
        totalHistogram[i] = 0;

    for (int i = 0; i < nrConsumers; i++) {
        unsigned int * histogram;
        pthread_join(consumer_threads[i], (void **) &histogram);

        for (int h = 0; h < maxval; h++)
            totalHistogram[h] += histogram[h];
        
        free(histogram);
    }

    int fd_out = open(argv[2], O_WRONLY | O_CREAT, 0644);
    for (int i = 0; i < maxval; i++) {
        char s[80];
        sprintf(s, "%d,%d\n", i, totalHistogram[i]);
        write(fd_out, s, strlen(s));
    }
    close(fd_out);

    free(producer_threads);
    free(consumer_threads);
    free(totalHistogram);
    free(nrBytesRead);
    free(Buffer);

    printf("Finished\n");
}