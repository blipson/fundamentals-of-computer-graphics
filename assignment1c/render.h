#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define EPSILON 1e-6

#include <float.h>

float distance(Vector3 v1, Vector3 v2) {
    return sqrtf(powf(v2.x - v1.x, 2.0f) + powf(v2.y - v1.y, 2.0f) + powf(v2.z - v1.z, 2.0f));
}

float magnitude(Vector3 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vector3 normalize(Vector3 v) {
    float m = magnitude(v);
    if (m != 0.0f) {
        return (Vector3) {
                .x = v.x / m,
                .y = v.y / m,
                .z = v.z / m
        };
    } else {
        return v;
    }
}

float normalizef(float value, float min, float max) {
    return value / (max - min);
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
            .y = (a.z * b.x) - (b.z * a.x),
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
            .y = v.y / c,
            .z = v.z / c
    };
}

void printVector(Vector3 v) {
    printf("{x: %lf, y: %lf, z: %lf}\n", v.x, v.y, v.z);
}

void printRGBColor(RGBColor c) {
    printf("{red: %hhu, green: %hhu, blue: %hhu}\n", c.red, c.green, c.blue);
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
    float halfWidth = viewParameters->viewingWindow.width / 2.0f;
    float halfHeight = viewParameters->viewingWindow.height / 2.0f;
    Vector3 widthTimesHorizontal = multiply(viewParameters->u, halfWidth);
    Vector3 heightTimesVertical = multiply(viewParameters->v, halfHeight);
    Vector3 eyePlusViewVector = add(scene.eye, viewDirTimesDistance);
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    viewParameters->viewingWindow.ul = add(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ur = add(perspectivePlusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    viewParameters->dh = divide(subtract(viewParameters->viewingWindow.ur, viewParameters->viewingWindow.ul),
                                ((float) scene.imSize.width - 1.0f));
    viewParameters->dv = divide(subtract(viewParameters->viewingWindow.ll, viewParameters->viewingWindow.ul),
                                ((float) scene.imSize.height - 1.0f));
}

void setParallelViewingWindow(Scene scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, viewParameters->n);
}

void setPerspectiveViewingWindow(Scene scene, ViewParameters* viewParameters) {
    setViewingWindowValues(scene, viewParameters, multiply(viewParameters->n, viewParameters->d));
}

void setViewingWindow(Scene scene, ViewParameters* viewParameters, bool parallel) {
    if (parallel) {
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

Ray traceRay(Scene scene, Vector3 viewingWindowLocation, bool parallel) {
    if (parallel) {
        return traceParallelRay(scene.viewDir, viewingWindowLocation);
    } else {
        return tracePerspectiveRay(scene.eye, viewingWindowLocation);
    }
}

RGBColor convertColorToRGBColor(Vector3 color) {
    return (RGBColor) {
            .red = convertFloatToUnsignedChar(min(color.x, 1)),
            .green = convertFloatToUnsignedChar(min(color.y, 1)),
            .blue = convertFloatToUnsignedChar(min(color.z, 1))
    };
}

Vector3 convertRGBColorToColor(RGBColor rgbColor) {
    return (Vector3) {
            .x = convertUnsignedCharToFloat(rgbColor.red) / 255.0f,
            .y = convertUnsignedCharToFloat(rgbColor.green) / 255.0f,
            .z = convertUnsignedCharToFloat(rgbColor.blue) / 255.0f,
    };
}

void checkSphereIntersection(Ray* ray, Scene* scene, int excludeIdx, int sphereIdx, float* closestIntersection, enum ObjectType* closestObject, int* closestSphereIdx) {
    if (sphereIdx != excludeIdx) {
        Sphere sphere = (*scene).spheres[sphereIdx];
        float A = dot((*ray).direction, (*ray).direction);
        float B = 2.0f * dot((*ray).direction, subtract((*ray).origin, sphere.center));
        float C = dot(subtract((*ray).origin, sphere.center), subtract((*ray).origin, sphere.center)) -
                  sphere.radius * sphere.radius;

        float discriminant = B * B - 4.0f * A * C;

        if (discriminant >= 0.0f) {
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2.0f * A);
            float t2 = (-B - sqrtDiscriminant) / (2.0f * A);

            if (t1 >= 0.0f && t1 < (*closestIntersection)) {
                (*closestIntersection) = t1;
                (*closestSphereIdx) = sphereIdx;
                (*closestObject) = SPHERE;
            }
            if (t2 >= 0.0f && t2 < (*closestIntersection)) {
                (*closestIntersection) = t2;
                (*closestSphereIdx) = sphereIdx;
                (*closestObject) = SPHERE;
            }
        }
    }
}

void checkSphereIntersections(int excludeIdx, Ray* ray, Scene* scene, float* closestIntersection, enum ObjectType* closestObject, int* closestSphereIdx) {
    for (int sphereIdx = 0; sphereIdx < (*scene).sphereCount; sphereIdx++) {
        checkSphereIntersection(ray, scene, excludeIdx, sphereIdx, closestIntersection, closestObject, closestSphereIdx);
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
    Vector3 n = cross(e1, e2);

    float D = -1.0f * dot(n, p0);

    float denominator = dot(n, (*ray).direction);
    if (fabsf(denominator) < EPSILON) {
        return -1;
    }

    float t = (-1.0f * (dot(n, (*ray).origin) + D)) / denominator;

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
    float A = magnitude(n);
    float alternativeAlpha = magnitude(divide(cross(subtract(p1, planeIntersectionPoint), subtract(p2, planeIntersectionPoint)), A));
    float alternativeBeta = magnitude(divide(cross(ep, e2), A));
    // you can use p0 instead of p1 in the second subtract, doesn't matter
    float alternativeGamma = magnitude(divide(cross(e1, subtract(planeIntersectionPoint, p1)), A));


    // Q: why do I have to do fabsf here? it's not in the slides but it's the only way it works
    if ((fabsf(alpha) + fabsf(beta) + fabsf(gamma)) < 1.0f + EPSILON) {
        if (t > 0.0f && t < (*closestIntersection)) {
            (*closestIntersection) = t;
            (*closestFaceIntersection) = (FaceIntersection) {
                .faceIdx = faceIdx,
                .n = n,
                .alpha = alpha,
                .beta = beta,
                .gamma = gamma,
            };
            (*closestObject) = TRIANGLE;
        }
    }
    return 0;
}

void checkFaceIntersections(Ray* ray, Scene* scene, float* closestIntersection, enum ObjectType* closestObject, FaceIntersection* closestFaceIntersection) {
    for (int faceIdx = 0; faceIdx < (*scene).faceCount; faceIdx++) {
        int earlyReturn = checkFaceIntersection(ray, scene, closestIntersection, closestObject, closestFaceIntersection, faceIdx);
        if (earlyReturn == -1) {
            continue;
        }
    }
}

Intersection castRay(Ray ray, Scene scene, int excludeIdx) {
    float closestIntersection = FLT_MAX; // Initialize with a large value
    enum ObjectType closestObject;
    int closestSphereIdx = -1;
    int closestEllipsoidIdx = -1;
    FaceIntersection closestFaceIntersection = (FaceIntersection) {
        .faceIdx = -1,
        .n = (Vector3) {
                .x = 0.0f,
                .y = 0.0f,
                .z = 0.0f,
        },
        .alpha = 0.0f,
        .beta = 0.0f,
        .gamma = 0.0f,
    };

    checkSphereIntersections(excludeIdx, &ray, &scene, &closestIntersection, &closestObject, &closestSphereIdx);

    checkEllipsoidIntersections(excludeIdx, &ray, &scene, &closestIntersection, &closestObject, &closestEllipsoidIdx);

    checkFaceIntersections(&ray, &scene, &closestIntersection, &closestObject, &closestFaceIntersection);

    return (Intersection) {
            .closestIntersection = closestIntersection,
            .closestSphereIdx = closestSphereIdx,
            .closestEllipsoidIdx = closestEllipsoidIdx,
            .closestFaceIntersection = closestFaceIntersection,
            .closestObject = closestObject
    };
}

float randomFloat() {
    return ((float) rand() / (float) RAND_MAX);
}

Vector3 randomUnitVector() {
    float z = 1.0f - 2.0f * randomFloat();
    float phi = 2.0f * (float) M_PI * randomFloat();
    float sqrtOneMinusZSquared = sqrtf(1.0f - z * z);

    float x = sqrtOneMinusZSquared * cosf(phi);
    float y = sqrtOneMinusZSquared * sinf(phi);

    return (Vector3) {x, y, z};
}

RGBColor shadeRay(Ray viewingRay, Scene scene, int x, int y) {
    Intersection intersection = castRay(viewingRay, scene, -1);
    Vector3 intersectionPoint = add(
            viewingRay.origin,
            multiply(
                    viewingRay.direction,
                    intersection.closestIntersection
            )
    );

    MaterialColor mtlColor;
    PPMImage texture;
    Vector3 surfaceNormal;

    if (intersection.closestSphereIdx != -1 && intersection.closestObject == SPHERE) {
        Sphere sphere = scene.spheres[intersection.closestSphereIdx];
        mtlColor = scene.mtlColors[sphere.mtlColorIdx];
        texture = scene.textures[sphere.textureIdx];
        surfaceNormal = normalize(divide(subtract(intersectionPoint, sphere.center), sphere.radius));
        if (texture.data != NULL) {
            float phi = acosf(surfaceNormal.z);
            float theta = atan2f(surfaceNormal.y, surfaceNormal.x);
            float v = phi / (float) M_PI;
            float u = max(theta/(2 * (float) M_PI), (theta + 2 * (float) M_PI) / (2 * (float) M_PI));
            mtlColor.diffuseColor = convertRGBColorToColor(texture.data[(int) (v * (float) texture.height)][(int) (u * (float) texture.width)]);
        }
    } else if (intersection.closestEllipsoidIdx != -1 && intersection.closestObject == ELLIPSOID) {
        Ellipsoid ellipsoid = scene.ellipsoids[intersection.closestEllipsoidIdx];
        mtlColor = scene.mtlColors[ellipsoid.mtlColorIdx];
        // todo: Calculate the surface normal for the ellipsoid
        surfaceNormal = normalize(divide(subtract(intersectionPoint, ellipsoid.center), ellipsoid.radius.x));
    } else if (intersection.closestFaceIntersection.faceIdx != -1 && intersection.closestObject == TRIANGLE) {
        Face face = scene.faces[intersection.closestFaceIntersection.faceIdx];
        mtlColor = scene.mtlColors[face.mtlColorIdx];

        if (scene.vertexNormals == NULL) {
            surfaceNormal = normalize(intersection.closestFaceIntersection.n);
        } else {
            Vector3 n0 = normalize(scene.vertexNormals[scene.faces[intersection.closestFaceIntersection.faceIdx].vn1 - 1]);
            Vector3 n1 = normalize(scene.vertexNormals[scene.faces[intersection.closestFaceIntersection.faceIdx].vn2 - 1]);
            Vector3 n2 = normalize(scene.vertexNormals[scene.faces[intersection.closestFaceIntersection.faceIdx].vn3 - 1]);
            Vector3 alphaComponent = multiply(n0, intersection.closestFaceIntersection.alpha);
            Vector3 betaComponent = multiply(n1, intersection.closestFaceIntersection.beta);
            Vector3 gammaComponent = multiply(n2, intersection.closestFaceIntersection.gamma);
            surfaceNormal = normalize(add(alphaComponent, add(betaComponent, gammaComponent)));
        }
    } else {
        return convertColorToRGBColor(scene.bkgColor);
    }

    Vector3 diffuseColor = mtlColor.diffuseColor;
    Vector3 specularColor = mtlColor.specularColor;
    Vector3 ambient = (Vector3) {
            .x = diffuseColor.x * mtlColor.ambientCoefficient,
            .y = diffuseColor.y * mtlColor.ambientCoefficient,
            .z = diffuseColor.z * mtlColor.ambientCoefficient,
    };
    Vector3 depthCueingAmbient = (Vector3) {
            .x = scene.depthCueing.color.x * mtlColor.ambientCoefficient,
            .y = scene.depthCueing.color.y * mtlColor.ambientCoefficient,
            .z = scene.depthCueing.color.z * mtlColor.ambientCoefficient,
    };

    Vector3 lightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 depthCueingLightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    if (scene.lightCount > 0) {
        for (int lightIdx = 0; lightIdx < scene.lightCount; lightIdx++) {
            Light light = scene.lights[lightIdx];
            Vector3 lightDirection = light.w == 1.0f
                                     ? normalize(subtract(light.position, intersectionPoint))
                                     : normalize(multiply(light.position, -1.0f));
            Vector3 intersectionToOrigin = normalize(subtract(scene.eye, intersectionPoint));
            Vector3 halfwayLightDirection = normalize(divide(add(lightDirection, intersectionToOrigin), 2.0f));
            float lightIntensity = light.i;
            Vector3 diffuse = (Vector3) {
                    .x = diffuseColor.x * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .y = diffuseColor.y * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .z = diffuseColor.z * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 depthCueingDiffuse = (Vector3) {
                    .x = scene.depthCueing.color.x * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .y = scene.depthCueing.color.y * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .z = scene.depthCueing.color.z * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 specular = (Vector3) {
                    .x =  specularColor.x * mtlColor.specularCoefficient *
                          powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .y = specularColor.y * mtlColor.specularCoefficient *
                         powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .z = specularColor.z * mtlColor.specularCoefficient *
                         powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
            };
            Vector3 depthCueingSpecular = (Vector3) {
                    .x =  scene.depthCueing.color.x * mtlColor.specularCoefficient *
                          powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .y = scene.depthCueing.color.y * mtlColor.specularCoefficient *
                         powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .z = scene.depthCueing.color.z * mtlColor.specularCoefficient *
                         powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
            };


            float shadow = 1.0f;
            if (scene.softShadows) {
                float softShadow = 0.0f;
                int numShadowRays = 500;

                for (int i = 0; i < numShadowRays; ++i) {
                    Vector3 jitteredLightDirection = add(lightDirection, multiply(randomUnitVector(), 0.005f));

                    Intersection shadowRay = castRay((Ray) {
                            .origin = intersectionPoint,
                            .direction = normalize(jitteredLightDirection)
                    }, scene, intersection.closestSphereIdx);

                    if (shadowRay.closestSphereIdx != -1) {
                        if ((light.w == 1.0f && shadowRay.closestIntersection < distance(intersectionPoint, light.position)) ||
                            (light.w == 0.0f && shadowRay.closestIntersection > 0.0f)) {
                            softShadow += 1.0f / (float) numShadowRays;
                        }
                    }
                }
                shadow = 1.0f - softShadow;
            } else {
                Intersection shadowRay = castRay((Ray) {
                        .origin = intersectionPoint,
                        .direction = lightDirection
                }, scene, intersection.closestSphereIdx);
                if (shadowRay.closestSphereIdx != -1) {
                    if (light.w == 1.0f && shadowRay.closestIntersection < distance(intersectionPoint, light.position)) {
                        shadow = 0.0f;
                    }
                    if (light.w == 0.0f && shadowRay.closestIntersection > 0.0f) {
                        shadow = 0.0f;
                    }
                }
            }

            if (scene.depthCueing.distMax > 0.0f) {
                float distanceToCamera = distance(scene.eye, intersectionPoint);
                float depthCueFactor = normalizef(distanceToCamera, scene.depthCueing.distMin, scene.depthCueing.distMax);
                diffuse = multiply(diffuse, 1.0f - depthCueFactor);
                specular = multiply(specular, 1.0f - depthCueFactor);
                ambient = multiply(specular, 1.0f - depthCueFactor);
                depthCueingDiffuse = multiply(depthCueingDiffuse, depthCueFactor);
                depthCueingSpecular = multiply(depthCueingSpecular, depthCueFactor);
                depthCueingAmbient = multiply(depthCueingAmbient, depthCueFactor);
            }
            float attenuationFactor = 1.0f;
            if (light.constantAttenuation > 0.0f || light.linearAttenuation > 0.0f || light.quadraticAttenuation > 0.0f) {
                attenuationFactor = 1.0f / (light.constantAttenuation + light.linearAttenuation * distance(intersectionPoint, light.position) + light.quadraticAttenuation * powf(distance(intersectionPoint, light.position), 2.0f));
            }
            Vector3 lightSourceAttenuationApplied = multiply(multiply(add(diffuse, specular), lightIntensity), attenuationFactor);
            Vector3 shadowsApplied = multiply(lightSourceAttenuationApplied, shadow);
            Vector3 depthCueingShadowsApplied = multiply(multiply(add(depthCueingDiffuse, depthCueingSpecular), lightIntensity), shadow);
            lightsApplied = add(lightsApplied, shadowsApplied);
            depthCueingLightsApplied = add(lightsApplied, depthCueingShadowsApplied);
        }
    }
    Vector3 ambientApplied = add(ambient, lightsApplied);
    Vector3 depthCueingAmbientApplied = add(depthCueingAmbient, depthCueingLightsApplied);
    return convertColorToRGBColor(add(ambientApplied, depthCueingAmbientApplied));
}

#endif
