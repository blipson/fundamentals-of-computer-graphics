#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

#include <math.h>

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

void printVector3(Vector3 v) {
    printf("{ %lf, %lf, %lf }\n", v.x, v.y, v.z);
}

Vector3 createRay(Scene scene, int x, int y) {
    // define the viewing parameters?
    // Q: why is it more computationally efficient to start with u and not v?
    // Q: why do we normalize w and u but not v?
    //     I guess v is already normalized because it's taking the cross product of two normalized vectors.
    //     why normalize at all though?
    // Q: why can't we use triangle rules with cos and sin and tan to define these?
    // Q: can you go over field of view ratio vs aspect ratio again?
    //     why can't we do 0.5 * fov.h when aspect ratio is 2?
    //     why can't we use pythagorean theorem on the fov?
    // Q: we don't ask the user to specify d. Why not?

    // step 1: n w u and v
    // define n: n = normalize(viewDir); will be used later...
    // define w: w = normalize(-viewDir);
    // compute u: u = normalize(cross(viewDir, upDir));
    // compute v as orthogonal to u and viewDir: v = cross(u, viewDir);
    Vector3 n = normalize(scene.viewDir);
    // w is never used
    Vector3 u = normalize(cross(scene.viewDir, scene.upDir));
    Vector3 v = cross(u, scene.viewDir);

    double aspectRatio = (double) scene.imSize.width / (double) scene.imSize.height;
    double halfWidth = tan(0.5 * (scene.fov.h * M_PI / 180.0));
    double halfHeight = halfWidth / aspectRatio;
    double d = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);

    // Is this right???
    scene.fov.v = 2.0 * atan(tan(0.5 * scene.fov.h * M_PI / 180.0) / aspectRatio) * (180.0 / M_PI);

    // step 2: width and height of the viewing window
    double width = 2 * d * tan(0.5 * scene.fov.h);
    double height = 2 * d * tan(0.5 * scene.fov.v);

    // step 3: 4 corners of the viewing window
    Vector3 ul = add(subtract(add(scene.eye, multiply(n, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    Vector3 ur = add(add(add(scene.eye, multiply(n, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    Vector3 ll = add(subtract(subtract(scene.eye, multiply(n, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    // lr is never used.

    // step 4: change in horizontal and vertical??
    Vector3 dh = divide(subtract(ur, ul), (scene.imSize.width - 1));
    Vector3 dv = divide(subtract(ll, ul), (scene.imSize.height - 1));
    Vector3 viewingWindowLocation = add(add(ul, multiply(dh, x)), multiply(dv, y));
    return viewingWindowLocation;
}



#endif
