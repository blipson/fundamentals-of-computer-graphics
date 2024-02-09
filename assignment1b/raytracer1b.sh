#!/bin/bash

directory="./tests/"

if [ -d "$directory" ]; then
    rm -f "$directory"*.ppm
    for file in "$directory"*.txt; do
        if [ -f "$file" ]; then
            ./raytracer1b "$file"
        else
            echo "Skipping non-regular file: $file"
        fi
    done
else
    echo "Directory $directory does not exist."
fi
