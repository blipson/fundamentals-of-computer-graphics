#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

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

Ray createRay(Scene scene, int x, int y) {
    // define the viewing parameters?
    // Q: why is it more computationally efficient to start with u and not v?
    // Q: why can't we use triangle rules with cos and sin and tan to define these?
    // Q: can you go over field of view ratio vs aspect ratio again?
    //     why can't we do 0.5 * fov.h when aspect ratio is 2?
    //     why can't we use pythagorean theorem on the fov?

    // step 1: w u and v
    Vector3 w = normalize(multiply(scene.viewDir, -1));
    Vector3 u = normalize(cross(w, scene.upDir));
    Vector3 v = cross(u, w);

    // step 2: d
    double aspectRatio = (double) scene.imSize.width / (double) scene.imSize.height;
    double halfWidth = tan(0.5 * (scene.fov.h * M_PI / 180.0));
    double halfHeight = halfWidth / aspectRatio;
    double d = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);

    // step 3: width and height of the viewing window
    double width = 2 * d * tan(0.5 * scene.fov.h);
    double height = 2 * d * tan(0.5 * scene.fov.v);

    // step 4: 4 corners of the viewing window
    Vector3 ul = add(subtract(add(scene.eye, multiply(w, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    Vector3 ur = add(add(add(scene.eye, multiply(w, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    Vector3 ll = add(subtract(subtract(scene.eye, multiply(w, d)), multiply(u, (width/2))), multiply(v, (height/2)));
    // Q: lr is never used??

    // step 5: change in horizontal and vertical??
    Vector3 dh = divide(subtract(ur, ul), (scene.imSize.width - 1));
    Vector3 dv = divide(subtract(ll, ul), (scene.imSize.height - 1));

    // step 6: find the viewing window location
    Vector3 viewingWindowLocation = add(add(ul, multiply(dh, x)), multiply(dv, y));
    Ray ray = {
            .origin = scene.eye,
            .direction = normalize(subtract(viewingWindowLocation, scene.eye))
    };
    return ray;
}

bool intersects(Ray ray, Sphere sphere) {
    Vector3 rayDir = normalize(subtract(ray.direction, ray.origin));
    double A = (rayDir.x * rayDir.x) + (rayDir.y * rayDir.y) + (rayDir.z * rayDir.z);
    double B = 2 * ((rayDir.x * (ray.origin.x - sphere.center.x)) + (rayDir.y * (ray.origin.y - sphere.center.y)) + (rayDir.z * (ray.origin.z - sphere.center.z)));
    double C = ((ray.origin.x - sphere.center.x) * (ray.origin.x - sphere.center.x)) + ((ray.origin.y - sphere.center.y) * (ray.origin.y - sphere.center.y)) + ((ray.origin.z - sphere.center.z) * (ray.origin.z - sphere.center.z)) - (sphere.radius * sphere.radius);

    double discriminant = B * B - 4 * A * C;

    if (discriminant >= 0) {
        double sqrtDiscriminant = sqrt(discriminant);
        double t1 = (-B + sqrtDiscriminant) / (2 * A);
        double t2 = (-B - sqrtDiscriminant) / (2 * A);

        if (t1 >= 0 || t2 >= 0) {
            return true;
        }
    }

    return false;
}

#endif
