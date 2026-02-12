#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc = 1) {
        sleep(2);
    }
    else {
        sleep(atoi(argv[1]));
    }
    
    printf("This is a process talking\n");

    return 0;
}