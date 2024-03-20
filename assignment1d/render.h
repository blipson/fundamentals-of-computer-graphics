#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define EPSILON 1e-2

#include <float.h>
#include "vector.h"
#include "ray.h"

RGBColor shadeRay(Ray ray, Scene scene, Exclusion exclusion, int reflectionDepth, float shadow, bool entering);

void applyLights(Scene scene, Intersection intersection, MaterialColor mtlColor, Vector3 surfaceNormal, Exclusion exclusion, float shadow, Vector3* ambient, Vector3* depthCueingAmbient, Vector3* lightsApplied, Vector3* depthCueingLightsApplied) {
    if (scene.lightCount > 0) {
        for (int lightIdx = 0; lightIdx < scene.lightCount; lightIdx++) {
            Light light = scene.lights[lightIdx];
            Vector3 lightDirection = light.pointOrDirectional == 1.0f
                                     ? normalize(subtract(light.position, intersection.intersectionPoint))
                                     : normalize(multiply(light.position, -1.0f));
            Vector3 intersectionToOrigin = normalize(subtract(scene.eye, intersection.intersectionPoint));
            Vector3 halfwayLightDirection = normalize(add(lightDirection, intersectionToOrigin));
            float lightIntensity = light.intensity;
            Vector3 diffuse = (Vector3) {
                    .x = mtlColor.diffuseColor.x * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .y = mtlColor.diffuseColor.y * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .z = mtlColor.diffuseColor.z * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 depthCueingDiffuse = (Vector3) {
                    .x = scene.depthCueing.color.x * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .y = scene.depthCueing.color.y * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
                    .z = scene.depthCueing.color.z * mtlColor.diffuseCoefficient * max(dot(surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 specular = (Vector3) {
                    .x =  mtlColor.specularColor.x * mtlColor.specularCoefficient *
                          powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .y = mtlColor.specularColor.y * mtlColor.specularCoefficient *
                         powf(max(dot(surfaceNormal, halfwayLightDirection), 0.0f), mtlColor.specularExponent),
                    .z = mtlColor.specularColor.z * mtlColor.specularCoefficient *
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

            // shadows
            if (scene.softShadows) {
                float softShadow = 0.0f;
                int numShadowRays = 500;

                for (int i = 0; i < numShadowRays; ++i) {
                    Vector3 jitteredLightDirection = add(lightDirection, multiply(randomUnitVector(), 0.005f));

                    Intersection shadowRay = castRay((Ray) {
                            .origin = intersection.intersectionPoint,
                            .direction = normalize(jitteredLightDirection)
                    }, scene, exclusion);

                    if (shadowRay.closestSphereIdx != -1 || shadowRay.closestEllipsoidIdx != -1 || shadowRay.closestFaceIntersection.faceIdx != -1) {
                        if ((light.pointOrDirectional == 1.0f && shadowRay.closestIntersection < distance(intersection.intersectionPoint, light.position)) ||
                            (light.pointOrDirectional == 0.0f && shadowRay.closestIntersection > 0.0f)) {
                            softShadow += 1.0f / (float) numShadowRays;
                        }
                    }
                }
                shadow = 1.0f - softShadow;
            } else {
                Intersection shadowIntersection = castRay((Ray) {
                        .origin = intersection.intersectionPoint,
                        .direction = lightDirection
                }, scene, exclusion);
                if (shadowIntersection.closestSphereIdx != -1 || shadowIntersection.closestEllipsoidIdx != -1 || shadowIntersection.closestFaceIntersection.faceIdx != -1) {
                    if (light.pointOrDirectional == 1.0f && shadowIntersection.closestIntersection < distance(intersection.intersectionPoint, light.position)) {
                        shadow *= (1.0f - shadowIntersection.mtlColor.alpha);
                    }
                    if (light.pointOrDirectional == 0.0f && shadowIntersection.closestIntersection > 0.0f) {
                        shadow *= (1.0f - shadowIntersection.mtlColor.alpha);
                    }
                }
            }

            // depth cueing
            if (scene.depthCueing.distMax > 0.0f) {
                float distanceToCamera = distance(scene.eye, intersection.intersectionPoint);
                float depthCueFactor = normalizef(distanceToCamera, scene.depthCueing.distMin, scene.depthCueing.distMax);
                diffuse = multiply(diffuse, 1.0f - depthCueFactor);
                specular = multiply(specular, 1.0f - depthCueFactor);
                *ambient = multiply(*ambient, 1.0f - depthCueFactor);
                depthCueingDiffuse = multiply(depthCueingDiffuse, depthCueFactor);
                depthCueingSpecular = multiply(depthCueingSpecular, depthCueFactor);
                *depthCueingAmbient = multiply(*depthCueingAmbient, depthCueFactor);
            }

            // attenuation
            float attenuationFactor = 1.0f;
            if (light.constantAttenuation > 0.0f || light.linearAttenuation > 0.0f || light.quadraticAttenuation > 0.0f) {
                attenuationFactor = 1.0f / (light.constantAttenuation + light.linearAttenuation * distance(intersection.intersectionPoint, light.position) + light.quadraticAttenuation * powf(distance(
                        intersection.intersectionPoint, light.position), 2.0f));
            }
            Vector3 lightSourceAttenuationApplied = multiply(multiply(add(diffuse, specular), lightIntensity), attenuationFactor);
            Vector3 shadowsApplied = multiply(lightSourceAttenuationApplied, shadow);
            Vector3 depthCueingShadowsApplied = multiply(multiply(add(depthCueingDiffuse, depthCueingSpecular), lightIntensity), shadow);
            *lightsApplied = add(*lightsApplied, shadowsApplied);
            *depthCueingLightsApplied = add(*lightsApplied, depthCueingShadowsApplied);
        }
    }
}

Vector3 applyBlinnPhongIllumination(
        Scene scene,
        Intersection intersection,
        int reflectionDepth,
        float shadow,
        bool entering
) {
    Exclusion exclusion = (Exclusion) {
        .excludeSphereIdx = intersection.closestSphereIdx,
        .excludeEllipsoidIdx = intersection.closestEllipsoidIdx,
        .excludeFaceIdx = intersection.closestFaceIntersection.faceIdx
    };

    Vector3 ambient = (Vector3) {
            .x = intersection.mtlColor.diffuseColor.x * intersection.mtlColor.ambientCoefficient,
            .y = intersection.mtlColor.diffuseColor.y * intersection.mtlColor.ambientCoefficient,
            .z = intersection.mtlColor.diffuseColor.z * intersection.mtlColor.ambientCoefficient,
    };
    Vector3 depthCueingAmbient = (Vector3) {
            .x = scene.depthCueing.color.x * intersection.mtlColor.ambientCoefficient,
            .y = scene.depthCueing.color.y * intersection.mtlColor.ambientCoefficient,
            .z = scene.depthCueing.color.z * intersection.mtlColor.ambientCoefficient,
    };

    Vector3 lightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 depthCueingLightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    applyLights(scene, intersection, intersection.mtlColor,intersection.surfaceNormal, exclusion, shadow, &ambient, &depthCueingAmbient, &lightsApplied, &depthCueingLightsApplied);


    Vector3 ambientApplied = add(ambient, lightsApplied);
    Vector3 depthCueingAmbientApplied = add(depthCueingAmbient, depthCueingLightsApplied);
    Vector3 illumination = add(ambientApplied, depthCueingAmbientApplied);

    Vector3 reflectionColor = illumination;
    Vector3 reverseIncidentDirection = multiply(intersection.incidentDirection, -1.0f);

    Vector3 reflection = (Vector3) {
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };

    float F0 = powf(((intersection.mtlColor.refractionIndex - 1.0f) / (intersection.mtlColor.refractionIndex + 1.0f)), 2);
    float Fr = F0 + ((1.0f - F0) * powf(1.0f - dot(reverseIncidentDirection, intersection.surfaceNormal), 5));

    if (intersection.mtlColor.specularCoefficient > 0.0f) {
        reflectionColor = convertRGBColorToColor(shadeRay(reflectRay(intersection.intersectionPoint, reverseIncidentDirection, intersection.surfaceNormal), scene, exclusion, reflectionDepth + 1, shadow, entering));
        reflection = multiply(reflectionColor, Fr);
        ambientApplied = multiply(ambientApplied, 1.0f - Fr);
        depthCueingAmbientApplied = multiply(depthCueingAmbientApplied, 1.0f - Fr);
        illumination = add(ambientApplied, depthCueingAmbientApplied);
    }

    Vector3 clampedIllumination = clamp(illumination);

    if (clampedIllumination.x == 0.0f && clampedIllumination.y == 0.0f && clampedIllumination.z == 0.0f) {
        reflection = (Vector3) {
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f
        };
    }

    Vector3 reflectionApplied = add(clampedIllumination, reflection);

    if (intersection.mtlColor.alpha >= 1.0f) {
        return reflectionApplied;
    }

    float currentRefractionIndex = scene.bkgColor.refractionIndex;
    float nextRefractionIndex = intersection.mtlColor.refractionIndex;

    if (!entering) {
        float tempRefractionIndex = currentRefractionIndex;
        currentRefractionIndex = nextRefractionIndex;
        nextRefractionIndex = tempRefractionIndex;
    }

    float cosThetaEntering = dot(intersection.surfaceNormal, intersection.incidentDirection);
    if (cosThetaEntering > 0.0f) {
        float tempRefractionIndex = currentRefractionIndex;
        currentRefractionIndex = nextRefractionIndex;
        nextRefractionIndex = tempRefractionIndex;
        intersection.surfaceNormal = multiply(intersection.surfaceNormal, -1.0f);
    }

    float refractionCoefficient = currentRefractionIndex / nextRefractionIndex;
    float partUnderSqrt = 1.0f - powf(refractionCoefficient, 2.0f) * (1.0f - powf(cosThetaEntering, 2.0f));
    if (partUnderSqrt < 0.0f) {
        return reflectionColor;
    }

    // Q: Why do I have to handle faces differently?
    Vector3 refractionDirToMultiply = intersection.closestObject == SPHERE ? subtract(multiply(intersection.surfaceNormal, cosThetaEntering), intersection.incidentDirection) : intersection.incidentDirection;

    float cosThetaExiting = sqrtf(partUnderSqrt);

    Ray nextIncident = (Ray) {
            .origin = intersection.intersectionPoint,
            .direction = add(
                    multiply(multiply(intersection.surfaceNormal, -1.0f),cosThetaExiting),
                    multiply(refractionDirToMultiply, refractionCoefficient)
            )
    };

    RGBColor refractionColor = shadeRay(nextIncident, scene, exclusion, reflectionDepth, shadow, !entering);

    Vector3 transparency = multiply(convertRGBColorToColor(refractionColor), ((1.0f - Fr) * (1.0f - intersection.mtlColor.alpha)));
    return add(reflectionApplied, transparency);

// somehow this code makes it work when the camera is in the sphere?
//    float cosThetaEntering = dot(surfaceNormal, incidentDirection);
//    if (cosThetaEntering > 0.0f) {
//        entering = !entering;
//        surfaceNormal = multiply(surfaceNormal, -1.0f);
//        cosThetaEntering = -cosThetaEntering;
//    }
//    float currentRefractionIndex = scene.bkgColor.refractionIndex;
//    float nextRefractionIndex = mtlColor.refractionIndex;
//    if (!entering) {
//        float tempRefractionIndex = currentRefractionIndex;
//        currentRefractionIndex = nextRefractionIndex;
//        nextRefractionIndex = tempRefractionIndex;
//        surfaceNormal = multiply(surfaceNormal, -1.0f);
//    }
//
//
//    float refractionCoefficient = currentRefractionIndex / nextRefractionIndex;
//    float partUnderSqrt = 1.0f - powf(refractionCoefficient, 2.0f) * (1.0f - powf(cosThetaEntering, 2.0f));
//    if (partUnderSqrt < 0.0f) {
//        return reflectionColor;
//    }
//
//    Vector3 refractionDirection = subtract(
//            multiply(incidentDirection, refractionCoefficient),
//            multiply(surfaceNormal, (refractionCoefficient * cosThetaEntering + sqrtf(partUnderSqrt)))
//    );
//
//    Vector3 refractionColor = convertRGBColorToColor(shadeRay(
//            (Ray){ .origin = intersectionPoint, .direction = normalize(refractionDirection) },
//            scene, rayInfo, reflectionDepth + 1, currentTransparency * mtlColor.alpha, shadow, entering
//    ));
//
//    F0 = powf(((currentRefractionIndex - nextRefractionIndex) / (currentRefractionIndex + nextRefractionIndex)), 2);
//    Fr = F0 + ((1.0f - F0) * powf(1.0f - dot(incidentDirection, surfaceNormal), 5));
//    reflectionColor = multiply(reflectionColor, Fr);
//    refractionColor = multiply(refractionColor, 1.0f - Fr);
//    return add(reflectionColor, refractionColor);
}

RGBColor shadeRay(Ray ray, Scene scene, Exclusion exclusion, int reflectionDepth, float shadow, bool entering) {
    if (reflectionDepth > 10) {
        return convertColorToRGBColor(scene.bkgColor.color);
    }

    Intersection intersection = castRay(ray, scene, exclusion);
    if (!intersectionExists(intersection)) {
        return convertColorToRGBColor(scene.bkgColor.color);
    }

    // TODO: How do I make transparency and refraction work with camera inside of sphere??
    // TODO: How do I make refraction work with underwater?
    // TODO: How do I make transparency and refraction work with multiple transparent objects nested inside each other?? Maintain a stack??

    return convertColorToRGBColor(applyBlinnPhongIllumination(scene, intersection, reflectionDepth, shadow, entering));
}

#endif
