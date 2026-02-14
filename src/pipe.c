#include "pipe.h"
#include "splitCommand.h"

#include <unistd.h>     //pipe, fork, dup2, close, execvp
#include <sys/wait.h>   //waitpid
#include <stdio.h>      //perror
#include <stdlib.h>     //malloc, free, exit
#include <string.h>     //strcpy, strlen

/* This code implements cmd1 | cmd2 by:
1. creating a pipe,
2. forking two children,
3. redirecting child1 stdout into pipe,
4. redirecting child2 stdin from pipe,
5. executing both commands with execvp,
6. and waiting for both with waitpid */

int run_piped(const char *cmd1, const char *cmd2){
    int fd[2];
    int pid1, pid2;

    //split_command modifies the string, so we copy cmd1/cmd2 so they can be written
    char *c1 = (char *)malloc(strlen(cmd1) + 1); //allocate enough space and \0
    char *c2 = (char *)malloc(strlen(cmd2) + 1);
    if(c1 == NULL || c2 == NULL){ //if it fails, cleam and return error -1
        free(c1);
        free(c2);
        return -1;
    }
    strcpy(c1, cmd1);
    strcpy(c2, cmd2);

    char **argv1 = split_command(c1); //convert the command strings into arguments
    char **argv2 = split_command(c2);
    if(argv1 == NULL || argv2 == NULL || argv1[0] == NULL || argv2[0] == NULL){
        free(argv1);
        free(argv2);
        free(c1);
        free(c2);
        return -1;
    }

    if(pipe(fd) < 0){
        perror("pipe");
        free(argv1); free(argv2);
        free(c1); free(c2);
        return -1;
    }

    //first child for output into the pipe (stdout -> fd[1])
    pid1 = fork();
    if(pid1 < 0){ //if <0 return error
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        free(argv1); free(argv2);
        free(c1); free(c2);
        return -1;
    }

    if(pid1 == 0){ //if == 0 we are in the child 
        dup2(fd[1], 1); //stdout becomes pipe write
        close(fd[0]);
        close(fd[1]);
        execvp(argv1[0], argv1);
        perror("execvp");
        exit(1);
    }

    //second child that gets input from the pipe (stdin <- fd[0])
    pid2 = fork();
    if(pid2 < 0){
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0); //to avoid a zombie
        free(argv1); free(argv2);
        free(c1); free(c2);
        return -1;
    }

    if(pid2 == 0){
        dup2(fd[0], 0); //stdin becomes pipe read
        close(fd[1]);
        close(fd[0]);
        execvp(argv2[0], argv2);
        perror("execvp");
        exit(1);
    }

    //parent close pipe and wait for both
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    //split_command returns argv arrays, but tokens point inside c1/c2
    free(argv1);
    free(argv2);
    free(c1);
    free(c2);

    return 0;
}
