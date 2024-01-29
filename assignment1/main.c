#include "input.h"
#include "output.h"
#include "render.h"

void render(FILE* outputFilePtr, int width, int height, Scene scene) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Ray ray = createRay(scene, x, y);
            writePixel(outputFilePtr, getPixelColor(ray, scene), x, width);
        }
    }
}

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char*** inputFileWordsByLine = readInputFile(argv[1]);

    // Default because the arrays must be instantiated with null to avoid seg faults
    Scene scene = {
            .eye = { .x = 0, .y = 0, .z = 0 },
            .viewDir = { .x = 0, .y = 0, .z = 0 },
            .upDir = { .x = 0, .y = 0, .z = 0 },
            .fov = { .h = 0 },
            .imSize = { .width = 0, .height = 0 },
            .bkgColor = { .red = 0, .green = 0, .blue = 0 },
            .parallel = { .frustumWidth = 0 },
            .mtlColors = NULL,
            .mtlColorCount = 0,
            .spheres = NULL,
            .sphereCount = 0,
            .ellipses = NULL,
            .ellipseCount = 0
    };

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene);
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    freeInputFileWordsByLine(inputFileWordsByLine);

//    printInput(scene);

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene.imSize.width, scene.imSize.height, scene);

    freeInput(scene);
    exit(0);
}
