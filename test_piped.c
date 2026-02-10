#include "pipe.h"

int main(void){
    run_piped("ls", "wc -l");
    return 0;
}
