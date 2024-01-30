#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

#include <float.h>

// Q: why did she use float instead of double?
double magnitude(Vector3 v) {
    return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vector3 normalize(Vector3 v) {
    double m = magnitude(v);
    if (m != 0.0) {
        Vector3 normalized = {
                .x = v.x / m,
                .y = v.y / m,
                .z = v.z / m
        };
        return normalized;
    } else {
        // What to do if the vector has 0 magnitude??
        return v;
    }
}

Vector3 multiply(Vector3 v, double s) {
    Vector3 multiplied = {
            .x = v.x * s,
            .y = v.y * s,
            .z = v.z * s
    };
    return multiplied;
}

Vector3 cross(Vector3 a, Vector3 b) {
    Vector3 crossed = {
            .x = (a.y * b.z) - (b.y * a.z),
            .y = (a.z * b.x) - (b.z* a.x),
            .z = (a.x * b.y) - (b.x * a.y)
    };
    return crossed;
}

double dot(Vector3 a, Vector3 b) {
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

Vector3 add(Vector3 a, Vector3 b) {
    Vector3 added = {
            .x = a.x + b.x,
            .y = a.y + b.y,
            .z = a.z + b.z
    };
    return added;
}

Vector3 subtract(Vector3 a, Vector3 b) {
    Vector3 subtracted = {
            .x = a.x - b.x,
            .y = a.y - b.y,
            .z = a.z - b.z
    };
    return subtracted;
}

Vector3 divide(Vector3 v, double c) {
    Vector3 divided = {
            .x = v.x / c,
            .y = v.y /c,
            .z = v.z / c
    };
    return divided;
}

void printVector(Vector3 v) {
    printf("%lf, %lf, %lf\n", v.x, v.y, v.z);
}

Ray createRay(Scene scene, int x, int y) {
    // Q: why is it more computationally efficient to start with u and not v?
    // Q: why can't we use triangle rules with cos and sin and tan to define these?
    // Q: can you go over field of view ratio vs aspect ratio again? why is it tan?
    //     why can't we do 0.5 * fov.h when aspect ratio is 2?
    //     why can't we use pythagorean theorem on the fov?

    // step 1: normalize the viewdir w, get the horizontal dir u and the vertical dir v
    Vector3 w = normalize(multiply(scene.viewDir, -1));
    Vector3 u = normalize(cross(w, scene.upDir));
    Vector3 v = cross(u, w);


    // step 2: calculate the distance away
    double aspectRatio = (double) scene.imSize.width / scene.imSize.height;

    // Q: how does d interact with halfWidth halfHeight width and height? How to properly calculate d?
    double d = 1.0;

    // step 3: width and height of the viewing window
    double width = 2 * d * tan(scene.fov.h / 2);
    double height = 2 * d * (tan(scene.fov.h / 2) / aspectRatio); // compute the height based on the width

    // step 4: 4 corners of the viewing window
    double halfWidth = width/2;
    double halfHeight = height/2;
    Vector3 widthTimesHorizontal = multiply(u, halfWidth);
    Vector3 heightTimesVertical = multiply(v, halfHeight);
    Vector3 eyePlusViewVector = add(scene.eye, multiply(w, d));
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    Vector3 ul = add(perspectiveMinusDimensions, heightTimesVertical);
    Vector3 ur = add(perspectivePlusDimensions, heightTimesVertical);
    Vector3 ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    Vector3 lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    // step 5: change in horizontal and vertical??
    Vector3 dh = divide(subtract(ur, ul), (scene.imSize.width - 1));
    Vector3 dv = divide(subtract(ll, ul), (scene.imSize.height - 1));

    // step 6: find the viewing window location
    Vector3 viewingWindowLocation = add(add(ul, multiply(dh, x)), multiply(dv, y));
    Ray ray = {
            // how do I test parallel projection with just spheres???
            .origin = scene.parallel.frustumWidth <= 0 ? scene.eye : normalize(viewingWindowLocation),
            .direction = normalize(subtract(scene.eye, viewingWindowLocation)),
    };
    return ray;
}

Vector3 inverse(Vector3 r) {
    Vector3 inversed = {
            .x = 1.0 / r.x,
            .y = 1.0 / r.y,
            .z = 1.0 / r.z
    };
    return inversed;
}

Vector3 square(Vector3 v) {
    Vector3 squared = {
            .x = v.x * v.x,
            .y = v.y * v.y,
            .z = v.z * v.z
    };
    return squared;
}

RGBColor getPixelColor(Ray ray, Scene scene) {
    // TODO: ellipses
    double closestIntersection = DBL_MAX; // Initialize with a large value
    bool closestIsSphere = false;
    int closestSphereIdx = -1;
    int closestEllipseIdx = -1;

    Vector3 rayDir = normalize(subtract(ray.direction, ray.origin));
    for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
        Sphere sphere = scene.spheres[sphereIdx];
        double A = dot(rayDir, rayDir);
        double B = 2 * dot(rayDir, subtract(ray.origin, sphere.center));
        double C = dot(subtract(ray.origin, sphere.center), subtract(ray.origin, sphere.center)) - sphere.radius * sphere.radius;

        double discriminant = B * B - 4 * A * C;

        if (discriminant >= 0) {
            double sqrtDiscriminant = sqrt(discriminant);
            double t1 = (-B + sqrtDiscriminant) / (2 * A);
            double t2 = (-B - sqrtDiscriminant) / (2 * A);

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

        double A = (ray.direction.x * ray.direction.x) / (ellipse.center.x * ellipse.center.x)
                   + (ray.direction.y * ray.direction.y) / (ellipse.center.y * ellipse.center.y)
                   + (ray.direction.z * ray.direction.z) / (ellipse.center.z * ellipse.center.z);


        double B = 2 * ((ray.direction.x * (ray.origin.x - ellipse.center.x)) / (ellipse.center.x * ellipse.center.x)
                        + (ray.direction.y * (ray.origin.y - ellipse.center.y)) / (ellipse.center.y * ellipse.center.y)
                        + (ray.direction.z * (ray.origin.z - ellipse.center.z)) / (ellipse.center.z * ellipse.center.z));

        double C = ((ray.origin.x - ellipse.center.x) * (ray.origin.x - ellipse.center.x)) / (ellipse.center.x * ellipse.center.x)
                   + ((ray.origin.y - ellipse.center.y) * (ray.origin.y - ellipse.center.y)) / (ellipse.center.y * ellipse.center.y)
                   + ((ray.origin.z - ellipse.center.z) * (ray.origin.z - ellipse.center.z)) / (ellipse.center.z * ellipse.center.z) - 1;


        double discriminant = B * B - 4 * A * C;

        if (discriminant >= 0) {
            double sqrtDiscriminant = sqrt(discriminant);
            double t1 = (-B + sqrtDiscriminant) / (2 * A);
            double t2 = (-B - sqrtDiscriminant) / (2 * A);

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
