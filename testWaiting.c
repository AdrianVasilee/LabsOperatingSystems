#include <stdio.h>
#include <unistd.h>

int main() {
    sleep(2);
    printf("This is a background process talking");

    return 0;
}