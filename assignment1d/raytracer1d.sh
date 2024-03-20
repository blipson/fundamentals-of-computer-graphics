#!/bin/bash

make

testDirectory="./tests/"
textureDirectory="./textures/"

if [ -d "$testDirectory" ]; then
    rm -f "$testDirectory"*.ppm
    cp "$textureDirectory"*.ppm "$testDirectory"
    for file in "$testDirectory"*.txt; do
        if [ -f "$file" ]; then
            echo "Rendering $file..."
            if [[ "$file" == "./tests/softshadows.txt" ]]; then
              ./raytracer1d -s "$file"
            else
              ./raytracer1d "$file"
            fi
        else
            echo "Skipping non-regular file: $file"
        fi
    done
else
    echo "Directory $testDirectory does not exist."
fi
