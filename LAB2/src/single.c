#include "single.h"

/* 
This code works for both Single and Concurrent execution
of programs
*/

int single(char *command, bool concurrent) {
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
        exit(0);
    }

    if (!concurrent) {
        waitpid(pid, NULL, 0);
    }

    free(args);
    free(cmd);

    return 0;
}