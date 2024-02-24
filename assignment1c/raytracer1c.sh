#!/bin/bash

make

directory="./tests/"

if [ -d "$directory" ]; then
    rm -f "$directory"*.ppm
    for file in "$directory"*.txt; do
        if [ -f "$file" ]; then
            echo "Rendering $file..."
            if [[ "$file" == "./tests/softshadows.txt" ]]; then
              echo "Skipping softshadows.txt..."
#              ./raytracer1c -s "$file"
            elif [[ "$file" != "./tests/showcase.txt" ]]; then
              ./raytracer1c "$file"
            fi
        else
            echo "Skipping non-regular file: $file"
        fi
    done
else
    echo "Directory $directory does not exist."
fi
