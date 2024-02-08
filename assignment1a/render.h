#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

#include <float.h>

float magnitude(Vector3 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vector3 normalize(Vector3 v) {
    float m = magnitude(v);
    if (m != 0.0) {
        return (Vector3) {
                .x = v.x / m,
                .y = v.y / m,
                .z = v.z / m
        };
    } else {
        // What to do if the vector has 0 magnitude??
        return v;
    }
}

Vector3 multiply(Vector3 v, float s) {
    return (Vector3) {
            .x = v.x * s,
            .y = v.y * s,
            .z = v.z * s
    };
}

Vector3 cross(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = (a.y * b.z) - (b.y * a.z),
            .y = (a.z * b.x) - (b.z* a.x),
            .z = (a.x * b.y) - (b.x * a.y)
    };
}

float dot(Vector3 a, Vector3 b) {
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

Vector3 add(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x + b.x,
            .y = a.y + b.y,
            .z = a.z + b.z
    };
}

Vector3 subtract(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x - b.x,
            .y = a.y - b.y,
            .z = a.z - b.z
    };
}

Vector3 divide(Vector3 v, float c) {
    return (Vector3) {
            .x = v.x / c,
            .y = v.y /c,
            .z = v.z / c
    };
}

void printVector(Vector3 v) {
    printf("{x: %lf, y: %lf, z: %lf}\n", v.x, v.y, v.z);
}

void printViewParameters(ViewParameters viewParameters) {
    printf("---------------VIEW PARAMETERS---------------\n");
    printf("w: ");
    printVector(viewParameters.w);
    printf("u: ");
    printVector(viewParameters.u);
    printf("v: ");
    printVector(viewParameters.v);
    printf("Aspect ratio: %lf\n", viewParameters.aspectRatio);
    printf("d: %lf\n", viewParameters.d);
    printf("width: %lf\n", viewParameters.viewingWindow.width);
    printf("height: %lf\n", viewParameters.viewingWindow.height);
    printf("ul: ");
    printVector(viewParameters.viewingWindow.ul);
    printf("ur: ");
    printVector(viewParameters.viewingWindow.ur);
    printf("ll: ");
    printVector(viewParameters.viewingWindow.ll);
    printf("lr: ");
    printVector(viewParameters.viewingWindow.lr);
    printf("---------------------------------------------\n");
}

void setViewingWindow(Scene scene, ViewParameters* viewParameters) {
    float halfWidth = viewParameters->viewingWindow.width/2;
    float halfHeight = viewParameters->viewingWindow.height/2;
    Vector3 widthTimesHorizontal = multiply(viewParameters->u, halfWidth);
    Vector3 heightTimesVertical = multiply(viewParameters->v, halfHeight);
    Vector3 eyePlusViewVector = add(scene.eye, multiply(viewParameters->n, viewParameters->d));
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    viewParameters->viewingWindow.ul = add(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ur = add(perspectivePlusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    viewParameters->dh = divide(subtract(viewParameters->viewingWindow.ur, viewParameters->viewingWindow.ul), ((float) scene.imSize.width - 1));
    viewParameters->dv = divide(subtract(viewParameters->viewingWindow.ll, viewParameters->viewingWindow.ul), ((float) scene.imSize.height - 1));
}

Ray createRay(Scene scene, ViewParameters viewParameters, int x, int y) {
    Vector3 viewingWindowLocation = add(
            add(
                    viewParameters.viewingWindow.ul,
                    multiply(
                            viewParameters.dh,
                            (float) x
                        )
                ),
                multiply(viewParameters.dv, (float) y)
    );

    if (scene.parallel.frustumWidth > 0) {
        return (Ray) {
            .origin = multiply(viewingWindowLocation, -1),
            .direction = normalize(scene.viewDir)
        };
    } else {
        return (Ray) {
                .origin = scene.eye,
                .direction = normalize(subtract(viewingWindowLocation, scene.eye))
        };
    }
}

RGBColor getPixelColor(Ray ray, Scene scene, int y, char* argv) {
    float closestIntersection = FLT_MAX; // Initialize with a large value
    bool closestIsSphere = false;
    int closestSphereIdx = -1;
    int closestEllipseIdx = -1;

    for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
        Sphere sphere = scene.spheres[sphereIdx];
        float A = dot(ray.direction, ray.direction);
        float B = 2 * dot(ray.direction, subtract(ray.origin, sphere.center));
        float C = dot(subtract(ray.origin, sphere.center), subtract(ray.origin, sphere.center)) - sphere.radius * sphere.radius;

        float discriminant = B * B - 4 * A * C;

        if (discriminant >= 0) {
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2 * A);
            float t2 = (-B - sqrtDiscriminant) / (2 * A);

            if (t1 >= 0 && t1 < closestIntersection) {
                closestIntersection = t1;
                closestSphereIdx = sphereIdx;
                closestIsSphere = true;
            }
            if (t2 >= 0 && t2 < closestIntersection) {
                closestIntersection = t2;
                closestSphereIdx = sphereIdx;
                closestIsSphere = true;
            }
        }
    }

    for (int ellipseIdx = 0; ellipseIdx < scene.ellipseCount; ellipseIdx++) {
        Ellipse ellipse = scene.ellipses[ellipseIdx];

        float A = (ray.direction.x * ray.direction.x) / (ellipse.radius.x * ellipse.radius.x)
                   + (ray.direction.y * ray.direction.y) / (ellipse.radius.y * ellipse.radius.y)
                   + (ray.direction.z * ray.direction.z) / (ellipse.radius.z * ellipse.radius.z);


        float B = 2 * ((ray.direction.x * (ray.origin.x - ellipse.center.x)) / (ellipse.radius.x * ellipse.radius.x)
                        + (ray.direction.y * (ray.origin.y - ellipse.center.y)) / (ellipse.radius.y * ellipse.radius.y)
                        + (ray.direction.z * (ray.origin.z - ellipse.center.z)) / (ellipse.radius.z * ellipse.radius.z));

        float C = ((ray.origin.x - ellipse.center.x) * (ray.origin.x - ellipse.center.x)) / (ellipse.radius.x * ellipse.radius.x)
                   + ((ray.origin.y - ellipse.center.y) * (ray.origin.y - ellipse.center.y)) / (ellipse.radius.y * ellipse.radius.y)
                   + ((ray.origin.z - ellipse.center.z) * (ray.origin.z - ellipse.center.z)) / (ellipse.radius.z * ellipse.radius.z) - 1;


        float discriminant = B * B - 4 * A * C;

        if ((strcmp(argv, "./showcase.txt") == 0 && discriminant >= 0 && discriminant <= 25) || discriminant >= 0) {
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2 * A);
            float t2 = (-B - sqrtDiscriminant) / (2 * A);

            if (t1 >= 0 && t1 < closestIntersection) {
                closestIntersection = t1;
                closestEllipseIdx = ellipseIdx;
                closestIsSphere = false;
            }
            if (t2 >= 0 && t2 < closestIntersection) {
                closestIntersection = t2;
                closestEllipseIdx = ellipseIdx;
                closestIsSphere = false;
            }
        }
    }

    if (closestSphereIdx != -1 && closestIsSphere) {
        Sphere sphere = scene.spheres[closestSphereIdx];
        if (strcmp(argv, "./showcase.txt") == 0) {
            if (sphere.center.z == -10) { // saturn
                return (RGBColor) {
                        .red = (y + 15) % 310,
                        .green = (y - 8) % 310,
                        .blue = y % 310,
                };
            } else if (sphere.center.z == -300) { // sun
                return (RGBColor) {
                        .red = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red,
                        .green = rand() % 255,
                        .blue = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue,
                };
            } else if (sphere.center.z == -500) { // neptune
                return (RGBColor) {
                        .red = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red,
                        .green = (y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].green) % 175,
                        .blue = (y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue) % 215
                };
            } else if (sphere.center.z == -150) { // mars
                return (RGBColor) {
                        .red = ((y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red) / 2) % 225,
                        .green = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].green,
                        .blue = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue
                };
            } else {
                return scene.mtlColors[sphere.mtlColorIdx];
            }
        } else {
            return scene.mtlColors[sphere.mtlColorIdx];
        }
    } else if (closestEllipseIdx != -1 && !closestIsSphere) {
        Ellipse ellipse = scene.ellipses[closestEllipseIdx];
        if (strcmp(argv, "./showcase.txt") == 0) {
            if (ellipse.center.z == -10) {
                return (RGBColor) {
                        .red = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].red) % 318,
                        .green = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].green) % 318,
                        .blue = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].blue) % 318,
                };
            } else {
                return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx];
            }
        } else {
            return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx];
        }
    } else {
        if (strcmp(argv, "./showcase.txt") == 0) {
            return rand() % 10000 > 1 ? scene.bkgColor : (RGBColor) {
                    .red = 255,
                    .green = 255,
                    .blue = 255
            };
        } else {
            return scene.bkgColor;
        }
    }
}

#endif