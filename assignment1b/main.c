#include "input.h"
#include "output.h"
#include "render.h"

void render(FILE* outputFilePtr, Scene scene, ViewParameters viewParameters, char* argv) {
    for (int y = 0; y < scene.imSize.height; y++) {
        for (int x = 0; x < scene.imSize.width; x++) {
            Ray ray = createRay(scene, viewParameters, x, y);
            writePixel(outputFilePtr, getPixelColor(ray, scene, y, argv), x, scene.imSize.width);
        }
    }
}

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char*** inputFileWordsByLine = readInputFile(argv[1]);

    // Default because the arrays must be instantiated with NULL to avoid seg faults
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
            .ellipseCount = 0,
            .lights = NULL,
    };

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene);
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    freeInputFileWordsByLine(inputFileWordsByLine);

    printScene(scene);

    ViewParameters viewParameters = {
            .w = normalize(multiply(scene.viewDir, -1)),
            .u = normalize(cross(scene.viewDir, scene.upDir)),
            .v = cross(viewParameters.u, normalize(scene.viewDir)),
            .n = normalize(scene.viewDir),
            .d = 1.0f,
            .aspectRatio = (float) scene.imSize.width / (float) scene.imSize.height,
            .viewingWindow = {
                    .width = scene.parallel.frustumWidth <= 0 ? 2 * viewParameters.d * tanf(scene.fov.h / 2) : scene.parallel.frustumWidth,
                    .height = scene.parallel.frustumWidth <= 0 ? 2 * viewParameters.d * (tanf(scene.fov.h / 2) / viewParameters.aspectRatio) : (scene.parallel.frustumWidth / viewParameters.aspectRatio),
            }
    };
    setViewingWindow(scene, &viewParameters);

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene, viewParameters, argv[1]);

    freeInput(scene);
    exit(0);
}
