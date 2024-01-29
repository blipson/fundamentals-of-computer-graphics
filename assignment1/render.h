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
    // Q: why is it more computationally efficient to start with u and not v?
    // Q: why can't we use triangle rules with cos and sin and tan to define these?
    // Q: can you go over field of view ratio vs aspect ratio again? why is it tan?
    //     why can't we do 0.5 * fov.h when aspect ratio is 2?
    //     why can't we use pythagorean theorem on the fov?

    // step 1: normalize the viewdir w, get the horizontal dir u and the vertical dir v
    Vector3 w = normalize(multiply(scene.viewDir, -1));
    Vector3 u = normalize(cross(scene.viewDir, scene.upDir));
    Vector3 v = cross(u, scene.viewDir);

    // step 2: calculate the distance away
    double aspectRatio = (double) scene.imSize.width / scene.imSize.height;

    // Q: how does d interact with halfWidth halfHeight width and height? How to properly calculate d?
    double d = 1.0;

    // step 3: width and height of the viewing window
    double width = 2 * d * tan(scene.fov.h / 2);
    double height = 2 * d * (aspectRatio * tan(scene.fov.h / 2)); // compute the height based on the width
    printf("(width, height): (%f, %f)\n", width, height);

    // step 4: 4 corners of the viewing window
    double halfWidth = width/2;
    double halfHeight = height/2;
    Vector3 widthTimesHorizontal = multiply(u, halfWidth);
    printf("widthTimesHorizontal: %f %f %f\n", widthTimesHorizontal.x, widthTimesHorizontal.y, widthTimesHorizontal.z);
    Vector3 heightTimesVertical = multiply(v, halfHeight);
    printf("heightTimesVertical: %f %f %f\n", heightTimesVertical.x, heightTimesVertical.y, heightTimesVertical.z);
    Vector3 eyePlusViewVector = add(scene.eye, multiply(w, d));
    printf("eyePlusViewVector: %f %f %f\n", eyePlusViewVector.x, eyePlusViewVector.y, eyePlusViewVector.z);
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    printf("perspectiveMinusDimensions: %f %f %f\n", perspectiveMinusDimensions.x, perspectiveMinusDimensions.y,perspectiveMinusDimensions.z);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);
    printf("perspectivePlusDimensions: %f %f %f\n", perspectivePlusDimensions.x, perspectivePlusDimensions.y, perspectivePlusDimensions.z);

    Vector3 ul = add(perspectiveMinusDimensions, heightTimesVertical);
    Vector3 ur = add(perspectivePlusDimensions, heightTimesVertical);
    Vector3 ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    Vector3 lr = subtract(perspectivePlusDimensions, heightTimesVertical); // never used??

//    printf("ul: %f %f %f\n", ul.x, ul.y, ul.z);
//    printf("ur: %f %f %f\n", ur.x, ur.y, ur.z);
//    printf("ll: %f %f %f\n", ll.x, ll.y, ll.z);
//    printf("lr: %f %f %f\n", lr.x, lr.y, lr.z);



    // step 5: change in horizontal and vertical??
    Vector3 test1 = subtract(ur, ul);
    Vector3 dh = divide(test1, (scene.imSize.width - 1));
    Vector3 test2 = subtract(ll, ul);
    Vector3 dv = divide(test2, (scene.imSize.height - 1));
    printf("dh: %f %f %f\n", dh.x, dh.y, dh.z);
    printf("dv: %f %f %f\n", dv.x, dv.y, dv.z);

    // step 6: find the viewing window location
    Vector3 viewingWindowLocation = add(add(ul, multiply(dh, x)), multiply(dv, y));
    Ray ray = {
            .origin = scene.eye,
            //.direction = normalize(subtract(viewingWindowLocation, scene.eye))
            .direction = normalize(subtract(scene.eye, viewingWindowLocation)),
    };
    return ray;
}

bool intersects(Ray ray, Sphere sphere) {
    // what if it intersects two??
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
