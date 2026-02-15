#!bin/bash

gcc src/main.c -o main

if [ $? -eq 0 ]; then
    echo "Compilation successful: main created."
else
    echo "Compilation failed."
fi