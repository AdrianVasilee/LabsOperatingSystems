#include "single.c"
#include "pipe.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define INPUT_SIZE 100

char *getInput() {
    char *inputBuffer = (char *)malloc(INPUT_SIZE); 
    int bytesRead = read(0, inputBuffer, INPUT_SIZE);
    inputBuffer[bytesRead - 1] = '\0'; // in order to get rid of the new line

    return inputBuffer;
}

int main() {

    while (1) {
        char *executionMode = getInput();

        if (strcmp(executionMode, "SINGLE") == 0) { 
            char* command = getInput();
            single(command, 0);

            free(command);
        } 

        else if (strcmp(executionMode, "PIPED") == 0) {
            char *command_1 = getInput(), *command_2 = getInput();
            run_piped(command_1, command_2);
            
            free(command_1);
            free(command_2);
        } 

        else if (strcmp(executionMode, "CONCURRENT") == 0) { 
            char* command = getInput();
            single(command, 1);

            free(command);
        } 
        
        else if (strcmp(executionMode, "EXIT") == 0) {
            break;
        } 
        
        else {
            char *output = "Execution mode not found\n";
            write(0, output, strlen(output));
        }

        free(executionMode);
    }

    return 0;
}