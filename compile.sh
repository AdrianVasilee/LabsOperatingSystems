#!/bin/bash

gcc -o main.exe src/main.c

if [ $? -eq 0 ]; then
    echo "Compilation successful: main.exe created."
else
    echo "Compilation failed."
fi