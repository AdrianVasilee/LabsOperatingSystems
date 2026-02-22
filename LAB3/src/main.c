#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "parsePGM.c"

typedef struct {
    char* path;
    int offset;
    int bytesToRead;
    // any other info you might want to pass
} ThreadInfo;

int maxval;

void * thread_function(void * info) {
    ThreadInfo * tInfo = (ThreadInfo *) info;

    unsigned int* histogram = malloc(maxval * sizeof(unsigned int));
    for (int i = 0; i < maxval; i++) {
        histogram[i] = 0;
    }

    int fd = open(tInfo->path, O_RDONLY);
    lseek(fd, tInfo->offset, SEEK_SET);
    unsigned char* buffer = malloc(tInfo->bytesToRead);

    int nBytesRead = read(fd, buffer, tInfo->bytesToRead);

    for (int i = 0; i < nBytesRead; i++)
        histogram[buffer[i]]++;

    close(fd);

    free(buffer);
    free(tInfo);

    return (void *) histogram;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Incorrect usage. Args needed: histogram pathToImage pathToHistogramOut numberOfThreads\n");
        _exit(1);
    }

    // Read the header
    int width, height;
    int nBytesHeader = parse_pgm_header(argv[1], &width, &height, &maxval);

    if (maxval > 255){
        perror("Expecting 1 byte ints\n");
        _exit(1);
    }

    int nPixels = width * height;

    // Starting threads
    int nr_threads = atoi(argv[3]);
    pthread_t *threads = malloc(sizeof(pthread_t) * nr_threads);

    int remainder = nPixels % nr_threads;
    int offset = nBytesHeader;

    for (int i = 0; i < nr_threads; i++){
        ThreadInfo *info = malloc(sizeof(ThreadInfo));
        info->offset = offset;
        info->path = argv[1];

        info->bytesToRead = nPixels/nr_threads;
        if (i < remainder)
            info->bytesToRead ++;

        // printf("The bytes that thread %d has to read is %d     Current offset: %d\n", i, info->bytesToRead, offset);

        offset += info->bytesToRead;

        pthread_create(&threads[i], NULL, thread_function, (void *) info);
    }

    // printf("Final offset %d, Bytes read %d\n", offset, offset - nBytesHeader);
    // printf("Total pixels it had to read %d\n", nPixels);

    // Computing the total histogram
    unsigned int * totalHistogram = malloc(maxval * sizeof(unsigned int));

    for (int i = 0; i < nr_threads; i++) {
        unsigned int * histogram;
        pthread_join(threads[i], (void **) &histogram);

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

    free(threads);
    free(totalHistogram);

    return 0;
}
