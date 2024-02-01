#include "input.h"
#include "output.h"
#include "render.h"

void render(FILE* outputFilePtr, Scene scene, ViewParameters viewParameters) {
    for (int y = 0; y < scene.imSize.height; y++) {
        for (int x = 0; x < scene.imSize.width; x++) {
            Ray ray = createRay(scene, viewParameters, x, y);
            writePixel(outputFilePtr, getPixelColor(ray, scene, x, y), x, scene.imSize.width);
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
            .ellipseCount = 0
    };

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene);
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    freeInputFileWordsByLine(inputFileWordsByLine);

    ViewParameters viewParameters = {
            .w = normalize(scene.viewDir),
            .u = normalize(cross(viewParameters.w, scene.upDir)),
            .v = cross(viewParameters.u, viewParameters.w),
            .aspectRatio = (float) scene.imSize.width / (float) scene.imSize.height,
            .d = 1.0f,
            .viewingWindow = {
                    .width = 2 * viewParameters.d * tanf(scene.fov.h / 2),
                    .height = 2 * viewParameters.d * (tanf(scene.fov.h / 2) / viewParameters.aspectRatio),
            }
    };
    setViewingWindow(scene, &viewParameters);

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene, viewParameters);

    freeInput(scene);
    exit(0);
}
