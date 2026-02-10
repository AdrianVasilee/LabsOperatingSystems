#include "pipe.h"

int main(void){
    run_piped("ls", "wc -l");
    //run_piped("ls", "grep this_should_not_exist"); shouldn't have any output
    //run_piped("yes", "head -n 5"); has to return 5 y 
    return 0;
}
