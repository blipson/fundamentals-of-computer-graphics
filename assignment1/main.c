#include "input.h"
#include "output.h"

// Q: why did she use float instead of double?

bool intersects(Vector3 ray, Sphere sphere, Vector3 eye) {
    // if (–B ± √(B^2 – 4C) ) / 2 is greater than or equal to 0 then true
    // else false

    Vector3 rayDir = normalize(subtract(ray, eye));
    double A = (rayDir.x * rayDir.x) + (rayDir.y * rayDir.y) + (rayDir.z * rayDir.z);
    double B = 2 * ((rayDir.x * (eye.x - sphere.center.x)) + (rayDir.y * (eye.y - sphere.center.y)) + (rayDir.z * (eye.z - sphere.center.z)));
    double C = ((eye.x - sphere.center.x) * (eye.x - sphere.center.x)) + ((eye.y - sphere.center.y) * (eye.y - sphere.center.y)) + ((eye.z - sphere.center.z) * (eye.z - sphere.center.z)) - (sphere.radius * sphere.radius);

    double discriminant = B * B - 4 * A * C;

    if (discriminant >= 0) {
        double sqrtDiscriminant = sqrt(discriminant);
        double t1 = (-B + sqrtDiscriminant) / (2 * A);
        double t2 = (-B - sqrtDiscriminant) / (2 * A);

        // Check if either t1 or t2 is greater than or equal to 0
        if (t1 >= 0 || t2 >= 0) {
            return true;
        }
    }

    return false;
}

void render(FILE* outputFilePtr, int width, int height, Scene scene) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector3 ray = createRay(scene, x, y);
            for (int mtlColorIdx = 0; mtlColorIdx < scene.mtlColorCount; mtlColorIdx++) {
                for (int sphereIdx = 0; sphereIdx < scene.mtlColors[mtlColorIdx].sphereCount; sphereIdx++) {
                    if (intersects(ray, scene.mtlColors[mtlColorIdx].spheres[sphereIdx], scene.eye)) {
                        writePixel(outputFilePtr, scene.mtlColors[mtlColorIdx].color, x, width);
                    } else {
                        writePixel(outputFilePtr, scene.bkgColor, x, width);
                    }
                }
            }
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
            .mtlColorCount = 0
    };

    int line = 0;
    readSceneSetup(inputFileWordsByLine, &line, &scene);
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    freeInputFileWordsByLine(inputFileWordsByLine);

    printInput(scene);

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    render(outputFilePtr, scene.imSize.width, scene.imSize.height, scene);

    freeInput(scene.mtlColors, scene.mtlColorCount);
    exit(0);
}
