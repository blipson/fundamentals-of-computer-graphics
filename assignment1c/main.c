#include "input.h"
#include "output.h"
#include "render.h"

void render(FILE* outputFilePtr, Scene scene, ViewParameters viewParameters) {
    for (int y = 0; y < scene.imSize.height; y++) {
        for (int x = 0; x < scene.imSize.width; x++) {
            Vector3 viewingWindowLocation = getViewingWindowLocation(viewParameters, x, y);
            Ray viewingRay = traceRay(scene, viewingWindowLocation);
            writePixel(outputFilePtr, shadeRay(viewingRay, scene), x, scene.imSize.width);
        }
    }
}

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);
    bool softShadows = strcmp(argv[1], "-s") == 0;

    char*** inputFileWordsByLine = readInputFile(argv, softShadows);

    Scene scene = {
            .eye = {.x = 0, .y = 0, .z = 0},
            .viewDir = {.x = 0, .y = 0, .z = 0},
            .upDir = {.x = 0, .y = 0, .z = 0},
            .fov = {.h = 0},
            .imSize = {.width = 0, .height = 0},
            .bkgColor = {.x = 0, .y = 0, .z = 0},
            .parallel = {.frustumWidth = 0},
            .mtlColors = (MaterialColor*) malloc(INITIAL_MTLCOLOR_COUNT * sizeof(MaterialColor)),
            .mtlColorCount = 0,
            .spheres = (Sphere*) malloc(INITIAL_SPHERE_COUNT * sizeof(Sphere)),
            .sphereCount = 0,
            .ellipses = (Ellipse*) malloc(INITIAL_ELLIPSE_COUNT * sizeof(Ellipse)),
            .ellipseCount = 0,
            .lights = (Light*) malloc(INITIAL_LIGHT_COUNT * sizeof(Light)),
            .lightCount = 0,
            .softShadows = false,
    };

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene, softShadows);
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    freeInputFileWordsByLine(inputFileWordsByLine);

    ViewParameters viewParameters = {
            .w = normalize(multiply(scene.viewDir, -1)),
            .u = normalize(cross(scene.viewDir, scene.upDir)),
            .v = cross(viewParameters.u, normalize(scene.viewDir)),
            .n = normalize(scene.viewDir),
            .d = 1.0f,
            .aspectRatio = (float) scene.imSize.width / (float) scene.imSize.height,
            .viewingWindow = {
                    .width = scene.parallel.frustumWidth <= 0 ?
                             (2 * viewParameters.d * tanf(scene.fov.h / 2)) :
                             (scene.parallel.frustumWidth),
                    .height = scene.parallel.frustumWidth <= 0 ?
                              (2 * viewParameters.d * (tanf(scene.fov.h / 2) / viewParameters.aspectRatio)) :
                              ((scene.parallel.frustumWidth / viewParameters.aspectRatio)),
            }
    };

    setViewingWindow(scene, &viewParameters);

    FILE* outputFilePtr = openOutputFile(softShadows ? argv[2] : argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene, viewParameters);

    freeInput(scene);

    exit(0);
}
