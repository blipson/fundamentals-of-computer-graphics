#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

#include <float.h>

float max(float a, float b) {
    return (a > b) ? a : b;
}

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

RGBColor multiplyRGB(RGBColor c, float s) {
    return (RGBColor) {
        .red = c.red * s,
        .green = c.green * s,
        .blue = c.blue * s
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

void setViewingWindowValues(Scene scene, ViewParameters* viewParameters, Vector3 viewDirTimesDistance) {
    float halfWidth = viewParameters->viewingWindow.width/2;
    float halfHeight = viewParameters->viewingWindow.height/2;
    Vector3 widthTimesHorizontal = multiply(viewParameters->u, halfWidth);
    Vector3 heightTimesVertical = multiply(viewParameters->v, halfHeight);
    Vector3 eyePlusViewVector = add(scene.eye, viewDirTimesDistance);
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    viewParameters->viewingWindow.ul = add(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ur = add(perspectivePlusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    viewParameters->dh = divide(subtract(viewParameters->viewingWindow.ur, viewParameters->viewingWindow.ul), ((float) scene.imSize.width - 1));
    viewParameters->dv = divide(subtract(viewParameters->viewingWindow.ll, viewParameters->viewingWindow.ul), ((float) scene.imSize.height - 1));
}

void setParallelViewingWindow(Scene scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, viewParameters->n);
}

void setPerspectiveViewingWindow(Scene scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, multiply(viewParameters->n, viewParameters->d));
}

void setViewingWindow(Scene scene, ViewParameters* viewParameters) {
    if (scene.parallel.frustumWidth > 0) {
        setParallelViewingWindow(scene, viewParameters);
    } else {
        setPerspectiveViewingWindow(scene, viewParameters);
    }
}

Vector3 getViewingWindowLocation(ViewParameters viewParameters, int x, int y) {
    return add(
            add(
                    viewParameters.viewingWindow.ul,
                    multiply(
                            viewParameters.dh,
                            (float) x
                    )
            ),
            multiply(viewParameters.dv, (float) y)
    );
}

Ray castParallelRay(Vector3 viewDir, Vector3 viewingWindowLocation) {
    return (Ray) {
            .origin = viewingWindowLocation,
            .direction = normalize(viewDir)
    };
}

Ray castPerspectiveRay(Vector3 eye, Vector3 viewingWindowLocation) {
    return (Ray) {
            .origin = eye,
            .direction = normalize(subtract(viewingWindowLocation, eye))
    };
}

Ray castRay(Scene scene, Vector3 viewingWindowLocation) {
    if (scene.parallel.frustumWidth > 0) {
        return castParallelRay(scene.viewDir, viewingWindowLocation);
    } else {
        return castPerspectiveRay(scene.eye, viewingWindowLocation);
    }
}

RGBColor traceRay(Ray ray, Scene scene) {
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

        if (discriminant >= 0) {
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
        return scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].diffuseColor;
    } else if (closestEllipseIdx != -1 && !closestIsSphere) {
        return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].diffuseColor;
    } else {
        return scene.bkgColor;
    }
}

#endif