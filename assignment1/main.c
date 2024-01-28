#include "input.h"
#include "output.h"
#include "render.h"

void render(FILE* outputFilePtr, int width, int height, Scene scene) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Ray ray = createRay(scene, x, y);
            for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
                if (intersects(ray, scene.spheres[sphereIdx])) {
                    writePixel(outputFilePtr, scene.mtlColors[scene.spheres[sphereIdx].mtlColorIdx], x, width);
                } else {
                    writePixel(outputFilePtr, scene.bkgColor, x, width);
                }
            }
            // TODO: ellipses
        }
    }
}

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char*** inputFileWordsByLine = readInputFile(argv[1]);

    // do we want to default?
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

    printInput(scene);

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene.imSize.width, scene.imSize.height, scene);

    freeInput(scene);
    exit(0);
}
