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
    Vector3 eyePlusViewVector = add(scene.eye, multiply(viewParameters->w, viewParameters->d));
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
    // Q: why is it more computationally efficient to start with u and not v?
    // Q: why can't we use triangle rules with cos and sin and tan to define these?
    // Q: can you go over field of view ratio vs aspect ratio again? why is it tan?
    //     why can't we do 0.5 * fov.h when aspect ratio is 2?
    //     why can't we use pythagorean theorem on the fov?

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
    Ray ray = {
            // how do I test parallel projection with just spheres???
            .origin = scene.parallel.frustumWidth <= 0 ? scene.eye : normalize(viewingWindowLocation),
            .direction = normalize(subtract(scene.eye, viewingWindowLocation)),
    };
    return ray;
}

Vector3 inverse(Vector3 r) {
    return (Vector3) {
            .x = 1.0f / r.x,
            .y = 1.0f / r.y,
            .z = 1.0f / r.z
    };
}

Vector3 square(Vector3 v) {
    return (Vector3) {
            .x = v.x * v.x,
            .y = v.y * v.y,
            .z = v.z * v.z
    };
}

RGBColor getPixelColor(Ray ray, Scene scene) {
    // TODO: ellipses
    float closestIntersection = FLT_MAX; // Initialize with a large value
    bool closestIsSphere = false;
    int closestSphereIdx = -1;
    int closestEllipseIdx = -1;

    Vector3 rayDir = normalize(subtract(ray.direction, ray.origin));
    for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
        Sphere sphere = scene.spheres[sphereIdx];
        float A = dot(rayDir, rayDir);
        float B = 2 * dot(rayDir, subtract(ray.origin, sphere.center));
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

    // I have no idea what I'm doing...
    for (int ellipseIdx = 0; ellipseIdx < scene.ellipseCount; ellipseIdx++) {
        Ellipse ellipse = scene.ellipses[ellipseIdx];

        float A = (ray.direction.x * ray.direction.x) / (ellipse.center.x * ellipse.center.x)
                   + (ray.direction.y * ray.direction.y) / (ellipse.center.y * ellipse.center.y)
                   + (ray.direction.z * ray.direction.z) / (ellipse.center.z * ellipse.center.z);


        float B = 2 * ((ray.direction.x * (ray.origin.x - ellipse.center.x)) / (ellipse.center.x * ellipse.center.x)
                        + (ray.direction.y * (ray.origin.y - ellipse.center.y)) / (ellipse.center.y * ellipse.center.y)
                        + (ray.direction.z * (ray.origin.z - ellipse.center.z)) / (ellipse.center.z * ellipse.center.z));

        float C = ((ray.origin.x - ellipse.center.x) * (ray.origin.x - ellipse.center.x)) / (ellipse.center.x * ellipse.center.x)
                   + ((ray.origin.y - ellipse.center.y) * (ray.origin.y - ellipse.center.y)) / (ellipse.center.y * ellipse.center.y)
                   + ((ray.origin.z - ellipse.center.z) * (ray.origin.z - ellipse.center.z)) / (ellipse.center.z * ellipse.center.z) - 1;


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
        return scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx];
    } else if (closestEllipseIdx != -1 && !closestIsSphere) {
        return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx];
    } else {
        return scene.bkgColor;
    }
}

#endif
