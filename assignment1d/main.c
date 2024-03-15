#include "input.h"
#include "output.h"
#include "render.h"

void progressBar(int total, int current) {
    const int barLength = 50;
    float progress = (float) current / (float) total;
    int barProgress = (int) (progress * (float) barLength);

    printf("[");
    for (int i = 0; i < barLength; ++i) {
        if (i < barProgress) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %.2f%%\r", progress * 100);
    fflush(stdout);
}

void render(FILE* outputFilePtr, Scene scene, ViewParameters viewParameters, bool parallel) {
    int i = 0;
    for (int y = 0; y < scene.imSize.height; y++) {
        for (int x = 0; x < scene.imSize.width; x++) {
            Vector3 viewingWindowLocation = getViewingWindowLocation(viewParameters, x, y);
            Ray viewingRay = traceRay(scene, viewingWindowLocation, parallel);
            writePixel(outputFilePtr, shadeRay(viewingRay, scene, 5, -1, -1, -1, x, y), x, scene.imSize.width);
            progressBar(scene.imSize.width * scene.imSize.height, i);
            i++;
        }
    }
}

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);
    bool softShadows = strcmp(argv[1], "-s") == 0;

    char*** inputFileWordsByLine = readInputFile(argv, softShadows);

    Scene scene = {
            .eye = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
            .viewDir = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
            .upDir = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
            .fov = {.h = 0.0f, .v = 0.0f },
            .imSize = {.width = 0, .height = 0},
            .bkgColor = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
            .parallel = {.frustumWidth = 0.0f},
            .mtlColors = (MaterialColor*) malloc(INITIAL_MTLCOLOR_COUNT * sizeof(MaterialColor)),
            .mtlColorCount = 0,
            .spheres = (Sphere*) malloc(INITIAL_SPHERE_COUNT * sizeof(Sphere)),
            .sphereCount = 0,
            .ellipsoids = (Ellipsoid*) malloc(INITIAL_ELLIPSOID_COUNT * sizeof(Ellipsoid)),
            .ellipsoidCount = 0,
            .lights = (Light*) malloc(INITIAL_LIGHT_COUNT * sizeof(Light)),
            .lightCount = 0,
            .softShadows = false,
            .vertexes = (Vector3*) malloc(INITIAL_VERTEX_COUNT * sizeof(Vector3)),
            .vertexCount = 0,
            .vertexNormals = NULL,
            .vertexNormalCount = 0,
            .vertexTextures = NULL,
            .vertexTextureCount = 0,
            .faces = (Face*) malloc(INITIAL_FACE_COUNT * sizeof(Face)),
            .faceCount = 0,
            .textures = (PPMImage*) malloc(INITIAL_TEXTURE_COUNT * sizeof (PPMImage)),
            .textureCount = 0,
            .normals = (PPMImage*) malloc(INITIAL_TEXTURE_COUNT * sizeof (PPMImage)),
            .normalCount = 0,
    };

    if (scene.mtlColors == NULL || scene.spheres == NULL || scene.ellipsoids == NULL || scene.lights == NULL || scene.faces == NULL || scene.textures == NULL || scene.normals == NULL) {
        fprintf(stderr, "Memory allocation failed for scene initialization.\n");
        exit(-1);
    }

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene, softShadows);
    readSceneObjects(inputFileWordsByLine, &line, &scene);
    freeInputFileWordsByLine(inputFileWordsByLine);
    bool parallel = scene.parallel.frustumWidth > 0.0f;
    bool horizontalFov = scene.fov.h > 0.0f;

    ViewParameters viewParameters = {
            .w = normalize(multiply(scene.viewDir, -1)),
            .u = normalize(cross(scene.viewDir, scene.upDir)),
            .v = cross(viewParameters.u, normalize(scene.viewDir)),
            .n = normalize(scene.viewDir),
            .d = 1.0f,
            .aspectRatio = (float) scene.imSize.width / (float) scene.imSize.height,
            .viewingWindow = {
                    .width = parallel ?
                             (scene.parallel.frustumWidth) :
                             horizontalFov ?
                             (2 * viewParameters.d * tanf(scene.fov.h / 2.0f)) :
                             (2 * viewParameters.d * tanf(scene.fov.v / 2.0f)) * viewParameters.aspectRatio,
                    .height = parallel ?
                              ((scene.parallel.frustumWidth / viewParameters.aspectRatio)) :
                              horizontalFov ?
                              (2 * viewParameters.d * (tanf(scene.fov.h / 2.0f) / viewParameters.aspectRatio)) :
                              (2 * viewParameters.d * (tanf(scene.fov.v / 2.0f))),
            }
    };

    setViewingWindow(scene, &viewParameters, parallel);

    FILE* outputFilePtr = openOutputFile(softShadows ? argv[2] : argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);

    render(outputFilePtr, scene, viewParameters, parallel);

    freeInput(scene);

    exit(0);
}
