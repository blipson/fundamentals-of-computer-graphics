# Assignment 0
This is a program to generate a valid output file in the ASCII PPM file format.

# Building
`$ make`

# Usage
`$ ./assignment0 path/to/input.txt <pattern>`

Pattern is an optional param that can be `[solid|grid|mandelbrot|julia|checkerboard|gaussian]`.

# Docs
The main function calls a number of helper functions. Documentation for those helper functions can be found here.

**NOTE: This documentation section is taking the place of explanatory comments baked within my code in order to stay in line with the [industry standard](https://bpoplauschi.github.io/2021/01/20/Clean-Code-Comments-by-Uncle-Bob-part-2.html) on code commenting.**

### Main
```c
int main(int argc, char* argv[]) {
    checkArgs(argc, argv); // Perform validation to ensure the command line args passed in are correct.

    // Read the optional pattern argument to determine which pattern to show.
    char* optionalPattern;
    if (argc == 3) {
        optionalPattern = argv[2];
    }

    // Read the input file contents.
    char inputFileContent[maxInputFileSize];
    getInputFileContent(argv[1], inputFileContent);
    char* fileContent[3];
    readFileContent(inputFileContent, fileContent);
    checkFileContent(fileContent); // Validate that it has the correct content.

    // Convert the width and height to valid positive integers.
    int width = convertStringToPositiveInt(fileContent[1]);
    int height = convertStringToPositiveInt(fileContent[2]);

    // Write to the output file and determine which pattern.
    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, width, height);
    if (argc == 2 || strcmp(optionalPattern, "solid") == 0) {
        writeSolidColorContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "grid") == 0) {
        writeGridContents(outputFilePtr, width, height);
    }
    // other patterns here

    // Close the file and exit the program.
    fclose(outputFilePtr);
    return 0;
}
```

### substr
```c
// Helper function to get the substring of a given string.
char* substr(char* s, int x, int y) {
    // Allocate the appropriate memory.
    char* ret = malloc(strlen(s) + 1);
    char* p = ret;
    
    // Get the starting character of the substring.
    char* q = &s[x];

    if (ret == NULL) {
        fprintf(stderr, "Can not copy string.");
        exit(-1);
    } else {
        while (x < y) {
            // Iterate until we get to the ending character of the substring.'
            *p++ = *q++;
            x++;
        }
        // Add a null terminating character at the end.
        *p = '\0';
    }
    return ret;
}
```

### checkArgs
```c
void checkArgs(int argc, char* argv[]) {
    if (argc > 3 || argc < 2) {
        // Too many or too few args passed in.
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./assignment0 <path/to/input_file.txt> <pattern>`");
        exit(-1);
    } else if (argc > 2) {
        // They passed in a 3rd optional param, validate that it's one of the expected values.
        if (strcmp(argv[2], "solid") != 0 && strcmp(argv[2], "grid") != 0 && strcmp(argv[2], "mandelbrot") != 0) {
            fprintf(stderr, "Incorrect usage. Pattern options are [solid, grid, mandelbrot].");
            exit(-1);
        }
    }

    if (strlen(argv[1]) > maxInputFileNameLength) {
        // Max input file length so they can't pass in an exceptionally large file.
        fprintf(stderr, "Input file name specified is too long. It must be under %d characters.", maxInputFileNameLength);
        exit(-1);
    }

    if (strcmp(substr(argv[1], (int) (strlen(argv[1]) - 4), (int) (strlen(argv[1]))), ".txt") != 0) {
        // Require .txt files.
        fprintf(stderr, "Incorrect input file format. Input file must be a '.txt' file.");
        exit(-1);
    }
}
```

### getInputFileContent
```c
void getInputFileContent(char* inputFileName, char* inputFileContents) {
    // Open the file.
    FILE* inputFilePtr;
    inputFilePtr = fopen(inputFileName, "r");

    // Check if the file exists.
    if (inputFilePtr != NULL) {
        // Get the stats on the file to be able to read the file size.
        struct stat inputFileStatus;
        stat(inputFileName, &inputFileStatus);

        if (inputFileStatus.st_size > maxInputFileSize) {
            // Handle if the file is too big.
            fprintf(stderr, "Input file is too big. It must be under %d characters.", maxInputFileSize);
            exit(-1);
        }

        // Get the file contents.
        fgets(inputFileContents, maxInputFileSize, inputFilePtr);

        // Handle extra content added to the end.
        char extraLines[maxInputFileSize];
        fgets(extraLines, maxInputFileSize, inputFilePtr);
        if (strlen(extraLines) > 0) {
            fprintf(stderr, "Improper file format. No extra lines should be included beyond 'imsize <width> <height>'");
            exit(-1);
        }
    } else {
        // Handle if the file doesn't exist.
        fprintf(stderr, "Unable to open the input file specified.");
        exit(-1);
    }
}
```

### readFileContent
```c
// Helper function to detect any of the different whitespace characters.
bool isWhiteSpace(char c) {
    return c == '\n' || c == '\t' || c == ' ';
}

void readFileContent(char* inputFileContents, char** fileContent) {
    // Expected format is `imsize <width> <height>`. Any other formats are considered invalid, including whitespace before or after.
    
    int i = 0;
    // Read the file until we get to the end of the first word.
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    // Get the first word.
    fileContent[0] = substr(inputFileContents, 0, i);

    // skip a single whitespace character. Multiple whitespace characters between words is considered invalid.
    i++;

    int widthStart = i;
    // Read the file until we get to the end of the second word.
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    // Get the second word.
    fileContent[1] = substr(inputFileContents, widthStart, i);

    // skip a single whitespace character. Multiple whitespace characters between words is considered invalid.
    i++;

    int heightStart = i;
    // Read the file until we get to the end of the third word.
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    // Get the third word.
    fileContent[2] = substr(inputFileContents, heightStart, i);

    // Handle any extra input at the end of the line.
    int extraStart = i;
    while (inputFileContents[i] != '\0') {
        i++;
    }
    char* extraInput = substr(inputFileContents, extraStart, i);
    if (strlen(extraInput) > 1) {
        fprintf(stderr, "Improper file format. No extra info should be included beyond 'imsize <width> <height>'.");
        exit(-1);
    }
}
```

### checkFileContent
```c
void checkFileContent(char** fileContent) {
    // Make sure the file content is valid.
    if (strcmp(fileContent[0], imSizeKeyword) != 0) {
        // Check the "imsize" keyword.
        fprintf(stderr, "Improper file format. The keyword 'imsize' must come first.");
        exit(-1);
    }

    // A maximum of 4 is chosen to have a max output size of 9999x9999 pixels. This supports all of the typical aspect ratios up to 7680x4320.
    if (strlen(fileContent[1]) > 4 || strlen(fileContent[1]) <= 0) {
        // Check the width.
        fprintf(stderr, "Improper file format. Width must be present and be max of 4 characters long.");
        exit(-1);
    }

    // A maximum of 4 is chosen to have a max output size of 9999x9999 pixels. This supports all of the typical aspect ratios up to 7680x4320.
    if (strlen(fileContent[2]) > 4 || strlen(fileContent[2]) <= 0) {
        // Check the height.
        fprintf(stderr, "Improper file format. Height must be present and be max of 4 characters long.");
        exit(-1);
    }
}
```

### convertStringToPositiveInt
```c
int convertStringToPositiveInt(char* s) {
    char* end;
    // Use strtol instead of atoi because atoi doesn't have any error handling for invalid values.
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        // if there is a nonzero number of non-number characters.
        fprintf(stderr, "Improper file format. Width and height must both be valid integers.");
        exit(-1);
    }
    if (i < 0) {
        // if the numbers passed in are negative.
        fprintf(stderr, "Improper file format. Width and height must both be positive integers.");
        exit(-1);
    }
    return i;
}
```

### openOutputFile
```c
FILE* openOutputFile(char* inputFileName) {
    // Dynamically determine the output file name based on the input file name.
    // Use substr to remove the ".txt" at the end.
    // Example: input.txt => input_output.txt
    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", substr(inputFileName, 0, strlen(inputFileName) - 4), outputFileSuffix);

    // Open the file with write permission to it.
    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}
```

### writeHeader
```c
void writeHeader(FILE* outputFilePtr, int width, int height) {
    // Example:
    // P3
    // 1920 1080
    // 255
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", magicNumber, width, height, maxColorComponentValue);
}
```

### writeSolidColorContents
```c
void enforceMaxPixelsOnLine(int x, FILE* outputFilePtr) {
    if (x % (maxPixelsOnLine) == (maxPixelsOnLine - 1)) {
        // If we've hit our max number of pixels, go to the next line.
        fprintf(outputFilePtr, "\n");
    } else {
        // Otherwise, put a tab between pixels for easy reading.
        fprintf(outputFilePtr, "\t");
    }
}

// This is the default option. It'll draw a solid color if they pass nothing in for the "pattern" param, or if they pass "solid".
void writeSolidColorContents(FILE* outputFilePtr, int width, int height) {
    // Loop through the height and the width and write a color to every pixel.
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // I chose yellow because I like yellow.
            fprintf(outputFilePtr, "255 222 111");
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}
```

### writeGridContents
```c
void enforceMaxPixelsOnLine(int x, FILE* outputFilePtr) {
    if (x % (maxPixelsOnLine) == (maxPixelsOnLine - 1)) {
        // If we've hit our max number of pixels, go to the next line.
        fprintf(outputFilePtr, "\n");
    } else {
        // Otherwise, put a tab between pixels for easy reading.
        fprintf(outputFilePtr, "\t");
    }
}

void writeGridContents(FILE* outputFilePtr, int width, int height) {
    // Use a boolean to track which color we're currently writing.
    bool firstColor = true;
    // Determine the square size based on the width. This allows it to scale to any resolution.
    // There will always be 10 total full squares.
    int squareSize = width/10;
    // We'll need to know if the width is divisible by 10 for the later logic.
    bool dividesWithoutRemainder = width%10 == 0;
    // Loop through the height and width.
    for (int y = 0; y < height; y++) {
        for (int x = 0; w < width; x++) {
            if ((x % squareSize == 0) && (dividesWithoutRemainder || x != 0)) {
                // Flip the color if we've reached our square size of 10 pixels only if the width divides cleanly.
                // 100 / 10 = 10, meaning each square will be 10 pixels wide for an image of width 100.
                // 101 / 10 = 10 R1, meaning each square will be 10 pixels wide, but there will be an extra slice of a square at the end, so we can't flip the color.
                // The last check for w != 0 handles flipping the color when you go to the next row.
                firstColor = !firstColor;
            }
            
            // Choose between the two colors based on the boolean.
            if (firstColor) {
                // I chose yellow because I like yellow.
                fprintf(outputFilePtr, "255 222 111");
            } else {
                // I chose blue because I like blue.
                fprintf(outputFilePtr, "0 0 200");
            }
            enforceMaxPixelsOnLine(w, outputFilePtr);
        }
        // Flip the color based on the rows as well so that we get a grid instead of lines.
        if (y % squareSize == 0) {
            firstColor = !firstColor;
        }
    }
}
```

### writeMandelbrotContents
```c
RGBColor mandelbrot(int x, int y, int width, int height, int maxIter) {
    // Map pixel coordinates to the complex plane
    double real = (x - width / 2.0) * 4.0 / width;
    double imaginary = (y - height / 2.0) * 4.0 / height;

    double realTemp, imagTemp;
    double realSquared, imagSquared;

    int i = 0;

    // Store initial values for later use
    realTemp = real;
    imagTemp = imaginary;
    
    while (i < maxIter) {
        realSquared = real * real - imaginary * imaginary;
        imagSquared = 2.0 * real * imaginary;

        real = realSquared + realTemp;
        imaginary = imagSquared + imagTemp;

        // Check if the point escapes the mandelbrot set
        if (real * real + imaginary * imaginary > 16.0) {
            break;
        }

        i++;
    }

    // Calculate smooth coloring using logarithmic transformations
    double squaredMagnitudeLog = log(real * real + imaginary * imaginary) / 2.0;
    double smoothIterationCount = log(squaredMagnitudeLog / log(2.0)) / log(2.0);
    i = (int)(i + 1 - smoothIterationCount);

    // Assign colors based on iteration
    RGBColor color;
    color.red = (i * 5) % 255;
    color.green = (i * 10) % 255;
    color.blue = (i * 20) % 255;

    // If max iterations reached, use black
    if (i >= maxIter) {
        RGBColor black;
        black.red = 0;
        black.green = 0;
        black.blue = 0;
        return black;
    }

    return color;
}

void writeMandelbrotContents(FILE* outputFilePtr, int width, int height) {
    // Loop through the height and width
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Get the pixel color in the mandelbrot set and write it
            RGBColor pixelColor = mandelbrot(x, y, width, height, 1000);
            fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}
```

### writeJuliaContents
```c
RGBColor julia(int x, int y, int width, int height, int maxIter, double realConstant, double imaginaryConstant) {
    // Map pixel coordinates to the complex plane
    double zx = 1.5 * (x - width / 2.0) / (0.5 * width);
    double zy = (y - height / 2.0) / (0.5 * height);

    double realSquared, imagSquared;

    int i = 0;

    while (i < maxIter) {
        realSquared = zx * zx - zy * zy;
        imagSquared = 2.0 * zx * zy;

        // Update the complex numbers based on the Julia set formula
        zx = realSquared + realConstant;
        zy = imagSquared + imaginaryConstant;

        // Check if the point has escaped the set
        if (realSquared + imagSquared > 16.0) {
            break;
        }

        i++;
    }

    // Assign colors based on the number of iterations
    RGBColor color;
    color.red = (i * 5) % 255;
    color.green = (i * 10) % 255;
    color.blue = (i * 20) % 255;

    return color;
}

void writeJuliaContents(FILE* outputFilePtr, int width, int height) {
    // Loop through the height and width
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Get the pixel color in the Julia set and write it
            // The constants -0.7 and 0.27015 were chosen to get a "classic" Julia set image
            RGBColor pixelColor = julia(x, y, width, height, 1000, -0.7, 0.27015);
            fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}
```