#ifndef __CONCURRENT__
#define __CONCURRENT__

#include "splitCommand.c"

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 

int single(char *command, bool concurrent);

#endif