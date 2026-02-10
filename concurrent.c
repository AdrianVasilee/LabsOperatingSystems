#include "concurrent.h"
#include "splitCommand.c"

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int concurrent(char *command) {
    char *cmd = (char *)malloc(strlen(command) + 1);

    strcpy(cmd, command);

    char **args = split_command(cmd);

    int pid = fork();
    
    if (pid < 0) {
        perror("Fork error");
        free(args);
        return -1;
    }

    if (pid == 0)  {
        execvp(args[0], args);

        free(args);
        free(cmd);
        return 0;
    }

    free(args);
    free(cmd);

    return 0;
}