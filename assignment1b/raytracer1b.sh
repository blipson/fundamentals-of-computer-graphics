#!/bin/bash

make

directory="./tests/"

if [ -d "$directory" ]; then
    rm -f "$directory"*.ppm
    for file in "$directory"*.txt; do
        if [ -f "$file" ]; then
            echo "Rendering $file..."
            if [[ "$file" == "./tests/softshadows.txt" ]]; then
              ./raytracer1b -s "$file"
            else
              ./raytracer1b "$file"
            fi
        else
            echo "Skipping non-regular file: $file"
        fi
    done
else
    echo "Directory $directory does not exist."
fi
