#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RAY_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RAY_H

#include "color.h"

void setViewingWindowValues(Scene* scene, ViewParameters* viewParameters, Vector3 viewDirTimesDistance) {
    float halfWidth = viewParameters->viewingWindow.width / 2.0f;
    float halfHeight = viewParameters->viewingWindow.height / 2.0f;
    Vector3 widthTimesHorizontal = multiply(viewParameters->u, halfWidth);
    Vector3 heightTimesVertical = multiply(viewParameters->v, halfHeight);
    Vector3 eyePlusViewVector = add(scene->eye, viewDirTimesDistance);
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    viewParameters->viewingWindow.ul = add(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ur = add(perspectivePlusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    viewParameters->dh = divide(subtract(viewParameters->viewingWindow.ur, viewParameters->viewingWindow.ul),
                                ((float) scene->imSize.width - 1.0f));
    viewParameters->dv = divide(subtract(viewParameters->viewingWindow.ll, viewParameters->viewingWindow.ul),
                                ((float) scene->imSize.height - 1.0f));
}

void setParallelViewingWindow(Scene* scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, viewParameters->n);
}

void setPerspectiveViewingWindow(Scene* scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, multiply(viewParameters->n, viewParameters->d));
}

void setViewingWindow(Scene* scene, ViewParameters* viewParameters, bool parallel) {
    if (parallel) {
        setParallelViewingWindow(scene, viewParameters);
    } else {
        setPerspectiveViewingWindow(scene, viewParameters);
    }
}

Vector3 getViewingWindowLocation(ViewParameters* viewParameters, int x, int y) {
    return add(
            add(
                    viewParameters->viewingWindow.ul,
                    multiply(
                            viewParameters->dh,
                            (float) x
                    )
            ),
            multiply(viewParameters->dv, (float) y)
    );
}

Ray traceParallelRay(Vector3 viewDir, Vector3 viewingWindowLocation) {
    return (Ray) {
            .origin = viewingWindowLocation,
            .direction = normalize(viewDir)
    };
}

Ray tracePerspectiveRay(Vector3 eye, Vector3 viewingWindowLocation) {
    return (Ray) {
            .origin = eye,
            .direction = normalize(subtract(viewingWindowLocation, eye))
    };
}

Ray traceViewingRay(Scene* scene, Vector3 viewingWindowLocation, bool parallel) {
    if (parallel) {
        return traceParallelRay(scene->viewDir, viewingWindowLocation);
    } else {
        return tracePerspectiveRay(scene->eye, viewingWindowLocation);
    }
}

void checkSphereIntersection(Ray* ray, Scene* scene, int excludeIdx, int sphereIdx, float* closestIntersection, enum ObjectType* closestObject, int* closestSphereIdx, bool bvh) {
    if (sphereIdx != excludeIdx) {
        Sphere sphere = bvh ? (*scene).bvhSpheres[sphereIdx] : (*scene).spheres[sphereIdx];
        float A = dot((*ray).direction, (*ray).direction);
        float B = 2.0f * dot((*ray).direction, subtract((*ray).origin, sphere.center));
        float C = dot(subtract((*ray).origin, sphere.center), subtract((*ray).origin, sphere.center)) -
                  sphere.radius * sphere.radius;

        float discriminant = B * B - 4.0f * A * C;

        if (discriminant >= 0.0f) {
            float smallDistance = distance((*ray).origin, addf((*ray).origin, EPSILON));

            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2.0f * A);
            float t2 = (-B - sqrtDiscriminant) / (2.0f * A);

            if (t1 >= 0.0f && t1 < (*closestIntersection) && t1 > smallDistance) {
                (*closestIntersection) = t1;
                (*closestSphereIdx) = sphereIdx;
                (*closestObject) = SPHERE;
            }
            if (t2 >= 0.0f && t2 < (*closestIntersection) && t2 > smallDistance) {
                (*closestIntersection) = t2;
                (*closestSphereIdx) = sphereIdx;
                (*closestObject) = SPHERE;
            }
        }
    }
}



void checkSphereIntersections(int excludeIdx, Ray* ray, Scene* scene, float* closestIntersection, enum ObjectType* closestObject, int* closestSphereIdx, bool bvh) {
    int sphereCount = bvh ? (*scene).bvhSphereCount : (*scene).sphereCount;
    for (int sphereIdx = 0; sphereIdx < sphereCount; sphereIdx++) {
        checkSphereIntersection(ray, scene, excludeIdx, sphereIdx, closestIntersection, closestObject, closestSphereIdx, bvh);
    }
}

void checkEllipsoidIntersection(Ray* ray, Scene* scene, int excludeIdx, int ellipsoidIdx, float* closestIntersection, enum ObjectType* closestObject, int* closestEllipsoidIdx) {
    if (ellipsoidIdx != excludeIdx) {
        Ellipsoid ellipsoid = (*scene).ellipsoids[ellipsoidIdx];

        float A = ((*ray).direction.x * (*ray).direction.x) / (ellipsoid.radius.x * ellipsoid.radius.x)
                  + ((*ray).direction.y * (*ray).direction.y) / (ellipsoid.radius.y * ellipsoid.radius.y)
                  + ((*ray).direction.z * (*ray).direction.z) / (ellipsoid.radius.z * ellipsoid.radius.z);


        float B = 2.0f * (((*ray).direction.x * ((*ray).origin.x - ellipsoid.center.x)) / (ellipsoid.radius.x * ellipsoid.radius.x)
                          +
                          ((*ray).direction.y * ((*ray).origin.y - ellipsoid.center.y)) / (ellipsoid.radius.y * ellipsoid.radius.y)
                          + ((*ray).direction.z * ((*ray).origin.z - ellipsoid.center.z)) /
                            (ellipsoid.radius.z * ellipsoid.radius.z));

        float C = (((*ray).origin.x - ellipsoid.center.x) * ((*ray).origin.x - ellipsoid.center.x)) /
                  (ellipsoid.radius.x * ellipsoid.radius.x)
                  + (((*ray).origin.y - ellipsoid.center.y) * ((*ray).origin.y - ellipsoid.center.y)) /
                    (ellipsoid.radius.y * ellipsoid.radius.y)
                  + (((*ray).origin.z - ellipsoid.center.z) * ((*ray).origin.z - ellipsoid.center.z)) /
                    (ellipsoid.radius.z * ellipsoid.radius.z) - 1.0f;


        float discriminant = B * B - 4.0f * A * C;

        if (discriminant >= 0.0f) {
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2.0f * A);
            float t2 = (-B - sqrtDiscriminant) / (2.0f * A);

            if (t1 >= 0.0f && t1 < (*closestIntersection)) {
                (*closestIntersection) = t1;
                (*closestEllipsoidIdx) = ellipsoidIdx;
                (*closestObject) = ELLIPSOID;
            }
            if (t2 >= 0.0f && t2 < (*closestIntersection)) {
                (*closestIntersection) = t2;
                (*closestEllipsoidIdx) = ellipsoidIdx;
                (*closestObject) = ELLIPSOID;
            }
        }
    }
}

void checkEllipsoidIntersections(int excludeIdx, Ray* ray, Scene* scene, float* closestIntersection, enum ObjectType* closestObject, int* closestEllipsoidIdx) {
    for (int ellipsoidIdx = 0; ellipsoidIdx < (*scene).ellipsoidCount; ellipsoidIdx++) {
        checkEllipsoidIntersection(ray, scene, excludeIdx, ellipsoidIdx, closestIntersection, closestObject, closestEllipsoidIdx);
    }
}

int checkFaceIntersection(const Ray* ray, const Scene* scene, float* closestIntersection, enum ObjectType* closestObject, FaceIntersection* closestFaceIntersection, int faceIdx) {
    Vector3 p0 = (*scene).vertexes[(*scene).faces[faceIdx].v1 - 1];
    Vector3 p1 = (*scene).vertexes[(*scene).faces[faceIdx].v2 - 1];
    Vector3 p2 = (*scene).vertexes[(*scene).faces[faceIdx].v3 - 1];

    Vector3 e1 = subtract(p1, p0);
    Vector3 e2 = subtract(p2, p0);
    Vector3 N = cross(e1, e2);

    float D = -1.0f * dot(N, p0);

    float denominator = dot(N, (*ray).direction);
    if (fabsf(denominator) < EPSILON) {
        return -1;
    }

    float t = (-1.0f * (dot(N, (*ray).origin) + D)) / denominator;

    Vector3 planeIntersectionPoint = add((*ray).origin, multiply((*ray).direction, t));

    float d11 = dot(e1, e1);
    float d12 = dot(e1, e2);
    float d22 = dot(e2, e2);

    Vector3 ep = subtract(planeIntersectionPoint, p0);
    float d1p = dot(e1, ep);
    float d2p = dot(e2, ep);

    float determinant = (d11 * d22) - (d12 * d12);
    if (fabsf(determinant) < EPSILON) {
        return -1;
    }

    float beta = (d22 * d1p - d12 * d2p) / determinant;
    float gamma = (d11 * d2p - d12 * d1p) / determinant;
    float alpha = 1.0f - beta - gamma;

    // Alternative method, what is this?
    // float A = magnitude(N);
    // float alternativeAlpha = magnitude(divide(cross(subtract(p1, planeIntersectionPoint), subtract(p2, planeIntersectionPoint)), A));
    // float alternativeBeta = magnitude(divide(cross(ep, e2), A));
    // you can use p0 instead of p1 in the second subtract, doesn't matter
    // float alternativeGamma = magnitude(divide(cross(e1, subtract(planeIntersectionPoint, p1)), A));


    if ((alpha > 0 && alpha < 1) && (beta > 0 && beta < 1) && (gamma > 0 && gamma < 1)) {
        float smallDistance = distance((*ray).origin, addf((*ray).origin, EPSILON));

        if (t >= 0.0f && t < (*closestIntersection) && t > smallDistance) {
            (*closestIntersection) = t;

            Vector3 T = (Vector3) { 0.0f, 0.0f, 0.0f };
            Vector3 B = (Vector3) { 0.0f, 0.0f, 0.0f };
            if ((*scene).vertexTextures != NULL) {
                float u0 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt1 - 1].u;
                float v0 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt1 - 1].v;
                float u1 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt2 - 1].u;
                float v1 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt2 - 1].v;
                float u2 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt3 - 1].u;
                float v2 = (*scene).vertexTextures[(*scene).faces[faceIdx].vt3 - 1].v;

                float deltaU1 = u1 - u0;
                float deltaU2 = u2 - u0;
                float deltaV1 = v1 - v0;
                float deltaV2 = v2 - v0;
                float d = 1.0f / (((-1.0f * deltaU1) * deltaV2) + (deltaU2 * deltaV1));
                T = normalize(multiply(add(multiply(e1, (-1.0f * deltaV2)), multiply(e2, deltaV1)), d));
                B = normalize(multiply(add(multiply(e1, (-1.0f * deltaU2)), multiply(e2, deltaU1)), d));
            }
            (*closestFaceIntersection) = (FaceIntersection) {
                    .faceIdx = faceIdx,
                    .normalDirection = N,
                    .alpha = alpha,
                    .beta = beta,
                    .gamma = gamma,
                    .tangentDirection = T,
                    .bitangentDirection = B,
            };
            (*closestObject) = TRIANGLE;
        }
    }
    return 0;
}

void checkFaceIntersections(int excludeIdx, Ray* ray, Scene* scene, float* closestIntersection, enum ObjectType* closestObject, FaceIntersection* closestFaceIntersection) {
    for (int faceIdx = 0; faceIdx < (*scene).faceCount; faceIdx++) {
        if (faceIdx != excludeIdx) {
            int earlyReturn = checkFaceIntersection(ray, scene, closestIntersection, closestObject, closestFaceIntersection, faceIdx);
            if (earlyReturn == -1) {
                continue;
            }
        }
    }
}

bool hasTextureData(PPMImage texture) { return texture.height > 0 && texture.width > 0 && texture.maxColor == 255 && texture.data != NULL; }

void handleSphereIntersection(Scene* scene, int closestSphereIdx, Vector3 intersectionPoint, MaterialColor* mtlColor, Vector3* surfaceNormal) {
    Sphere sphere = scene->spheres[closestSphereIdx];
    (*mtlColor) = scene->mtlColors[sphere.mtlColorIdx];
    PPMImage texture = scene->textures[sphere.textureIdx];
    PPMImage normal = scene->normals[sphere.normalIdx];
    (*surfaceNormal) = normalize(divide(subtract(intersectionPoint, sphere.center), sphere.radius));
    if (hasTextureData(texture)) {
        float phi = acosf((*surfaceNormal).z);
        float theta = atan2f((*surfaceNormal).y, (*surfaceNormal).x);
        float v = phi / (float) M_PI;
        float u = max(theta/(2.0f * (float) M_PI), (theta + 2.0f * (float) M_PI) / (2.0f * (float) M_PI));
        int x = (int) roundf(u * (float) (texture.width-1)) % (texture.width - 1);
        int y = (int) roundf(v * (float) (texture.height-1)) % (texture.height - 1);

        if (normal.height > 0 && normal.width > 0 && normal.maxColor == 255 && normal.data != NULL) {
            Vector3 normalMatrix = normalize(convertNormalToVector(normal.data[y][x]));
            Vector3 normalDirection = normalize((Vector3) {
                    .x = cosf(theta) * sinf(phi),
                    .y = sinf(theta) * sinf(phi),
                    .z = cosf(phi),
            });
            Vector3 tangentDirection = normalize((Vector3) {
                    .x = (-1.0f * normalDirection.y) / (sqrtf((normalDirection.x * normalDirection.x) + (normalDirection.y * normalDirection.y))),
                    .y = (normalDirection.x) / (sqrtf((normalDirection.x * normalDirection.x) + (normalDirection.y * normalDirection.y))),
                    .z = 0.0f,
            });
            Vector3 bitangentDirection = (Vector3) {
                    .x = (-1.0f * normalDirection.z) * tangentDirection.y,
                    .y = normalDirection.z * tangentDirection.x,
                    .z = sqrtf((normalDirection.x * normalDirection.x) + (normalDirection.y * normalDirection.y)),
            };

            (*surfaceNormal) = tangentSpaceToWorldSpace(normalMatrix, tangentDirection, bitangentDirection, (*surfaceNormal));
        }
        (*mtlColor).diffuseColor = convertRGBColorToColor(texture.data[y][x]);
    }
}


void handleEllipsoidIntersection(Scene* scene, int closestEllipsoidIdx, Vector3 intersectionPoint, MaterialColor* mtlColor, Vector3* surfaceNormal) {
    Ellipsoid ellipsoid = scene->ellipsoids[closestEllipsoidIdx];
    (*mtlColor) = scene->mtlColors[ellipsoid.mtlColorIdx];
    // todo: Calculate the surface normal for the ellipsoid
    (*surfaceNormal) = normalize(divide(subtract(intersectionPoint, ellipsoid.center), ellipsoid.radius.x));
}

Vector3 applyBilinearInterpolation(FaceIntersection intersection, RGBColor** textureData, int x, int y) {
    return add(
            add(
                    add(
                            multiply(
                                    multiply(
                                            convertRGBColorToColor(textureData[y][x]),
                                            (1 - intersection.alpha)
                                    ),
                                    (1 - intersection.beta)
                            ),
                            multiply(
                                    multiply(
                                            convertRGBColorToColor(textureData[y][x + 1]),
                                            (intersection.alpha)
                                    ),
                                    (1 - intersection.beta)
                            )
                    ),
                    multiply(
                            multiply(
                                    convertRGBColorToColor(textureData[y + 1][x]),
                                    (1 - intersection.alpha)
                            ),
                            (intersection.beta)
                    )
            ),
            multiply(
                    multiply(
                            convertRGBColorToColor(textureData[y + 1][x + 1]),
                            (intersection.alpha)
                    ),
                    (intersection.beta)
            )
    );
}

void handleFaceIntersection(Scene* scene, FaceIntersection closestFaceIntersection, MaterialColor* mtlColor, Vector3* surfaceNormal) {
    Face face = scene->faces[closestFaceIntersection.faceIdx];
    PPMImage texture = scene->textures[face.textureIdx];
    PPMImage normal = scene->normals[face.normalIdx];
    (*mtlColor) = scene->mtlColors[face.mtlColorIdx];

    if (scene->vertexNormals == NULL) {
        (*surfaceNormal) = normalize(closestFaceIntersection.normalDirection);
    } else {
        Vector3 n0 = normalize(scene->vertexNormals[scene->faces[closestFaceIntersection.faceIdx].vn1 - 1]);
        Vector3 n1 = normalize(scene->vertexNormals[scene->faces[closestFaceIntersection.faceIdx].vn2 - 1]);
        Vector3 n2 = normalize(scene->vertexNormals[scene->faces[closestFaceIntersection.faceIdx].vn3 - 1]);
        Vector3 alphaComponent = multiply(n0, closestFaceIntersection.alpha);
        Vector3 betaComponent = multiply(n1, closestFaceIntersection.beta);
        Vector3 gammaComponent = multiply(n2, closestFaceIntersection.gamma);
        (*surfaceNormal) = normalize(add(alphaComponent, add(betaComponent, gammaComponent)));
    }

    if (scene->vertexTextures != NULL && hasTextureData(texture)) {
        float u0 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt1 - 1].u;
        float v0 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt1 - 1].v;
        float u1 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt2 - 1].u;
        float v1 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt2 - 1].v;
        float u2 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt3 - 1].u;
        float v2 = scene->vertexTextures[scene->faces[closestFaceIntersection.faceIdx].vt3 - 1].v;

        float u = closestFaceIntersection.alpha * u0 + closestFaceIntersection.beta * u1 + closestFaceIntersection.gamma * u2;
        float v = closestFaceIntersection.alpha * v0 + closestFaceIntersection.beta * v1 + closestFaceIntersection.gamma * v2;

        float uInt;
        float uFractional = modff(u, &uInt);

        float vInt;
        float vFractional = modff(v, &vInt);

        int x = (int) roundf(uFractional * ((float) texture.width - 1.0f)) % (texture.width - 1);
        int y = (int) roundf(vFractional * ((float) texture.height - 1.0f)) % (texture.height - 1);

        if (normal.height > 0 && normal.width > 0 && normal.maxColor == 255 && normal.data != NULL) {
            Vector3 normalMatrix = normalize(convertNormalToVector(normal.data[y][x]));
            (*surfaceNormal) = tangentSpaceToWorldSpace(normalMatrix, closestFaceIntersection.tangentDirection, closestFaceIntersection.bitangentDirection, (*surfaceNormal));

        }

        (*mtlColor).diffuseColor = applyBilinearInterpolation(closestFaceIntersection, texture.data, x, y);
    }
}

bool intersectionExists(Intersection intersection) {
    return (intersection.closestSphereIdx != -1 && intersection.closestObject == SPHERE) ||
        (intersection.closestEllipsoidIdx != -1 && intersection.closestObject == ELLIPSOID) ||
        (intersection.closestFaceIntersection.faceIdx != -1 && intersection.closestObject == TRIANGLE);
}

Ray reflectRay(Vector3 intersectionPoint, Vector3 reverseIncidentDirection, Vector3 surfaceNormal) {
    return (Ray) {
            .origin = intersectionPoint,
            .direction = subtract((multiply(surfaceNormal, (2.0f * dot(surfaceNormal, reverseIncidentDirection)))), reverseIncidentDirection)
    };
}

Intersection castBvhRay(Ray ray, Scene* scene) {
    float closestIntersection = FLT_MAX; // Initialize with a large value
    enum ObjectType closestObject;
    int closestBvhSphereIdx = -1;
    checkSphereIntersections(-1, &ray, scene, &closestIntersection, &closestObject, &closestBvhSphereIdx, true);
    return (Intersection) {
            .closestIntersection = closestIntersection,
            .closestSphereIdx = closestBvhSphereIdx,
            .closestEllipsoidIdx = -1,
            .closestFaceIntersection = (FaceIntersection) {
                .faceIdx = -1,
                .normalDirection = (Vector3) {
                        .x = 0.0f,
                        .y = 0.0f,
                        .z = 0.0f,
                },
                .alpha = 0.0f,
                .beta = 0.0f,
                .gamma = 0.0f,
            },
            .closestObject = closestObject,
    };
}

Intersection castRay(Ray ray, Scene* scene, Exclusion exclusion) {
    float closestIntersection = FLT_MAX; // Initialize with a large value
    enum ObjectType closestObject;
    int closestSphereIdx = -1;
    int closestEllipsoidIdx = -1;
    FaceIntersection closestFaceIntersection = (FaceIntersection) {
            .faceIdx = -1,
            .normalDirection = (Vector3) {
                    .x = 0.0f,
                    .y = 0.0f,
                    .z = 0.0f,
            },
            .alpha = 0.0f,
            .beta = 0.0f,
            .gamma = 0.0f,
    };
    checkSphereIntersections(exclusion.excludeSphereIdx, &ray, scene, &closestIntersection, &closestObject, &closestSphereIdx, false);

    checkEllipsoidIntersections(exclusion.excludeEllipsoidIdx, &ray, scene, &closestIntersection, &closestObject, &closestEllipsoidIdx);

    checkFaceIntersections(exclusion.excludeFaceIdx, &ray, scene, &closestIntersection, &closestObject, &closestFaceIntersection);

    Vector3 intersectionPoint = add(
            ray.origin,
            multiply(
                    ray.direction,
                    closestIntersection
            )
    );

    MaterialColor mtlColor;
    Vector3 surfaceNormal;

    if (closestSphereIdx != -1 && closestObject == SPHERE) {
        handleSphereIntersection(scene, closestSphereIdx, intersectionPoint, &mtlColor, &surfaceNormal);
    } else if (closestEllipsoidIdx != -1 && closestObject == ELLIPSOID) {
        handleEllipsoidIntersection(scene, closestEllipsoidIdx, intersectionPoint, &mtlColor, &surfaceNormal);
    } else if (closestFaceIntersection.faceIdx != -1 && closestObject == TRIANGLE) {
        handleFaceIntersection(scene, closestFaceIntersection, &mtlColor, &surfaceNormal);
    } else {
        // didn't hit anything
    }

    return (Intersection) {
            .closestIntersection = closestIntersection,
            .closestSphereIdx = closestSphereIdx,
            .closestEllipsoidIdx = closestEllipsoidIdx,
            .closestFaceIntersection = closestFaceIntersection,
            .closestObject = closestObject,
            .intersectionPoint = intersectionPoint,
            .surfaceNormal = surfaceNormal,
            .mtlColor = mtlColor,
            .incidentDirection = ray.direction,
            .exclusion = (Exclusion) {
                .excludeSphereIdx = closestSphereIdx,
                .excludeEllipsoidIdx = closestEllipsoidIdx,
                .excludeFaceIdx = closestFaceIntersection.faceIdx
            }
    };
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

#endif