#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define EPSILON 1e-2

#include <float.h>
#include "vector.h"
#include "ray.h"

#define MAX_REFLECTION_DEPTH 3

Vector3 shadeRay(Ray ray, Scene* scene, RayState rayState);

Illumination applyLights(Scene* scene, Intersection intersection, float shadow) {
    Vector3 ambient = (Vector3) {
            .x = intersection.mtlColor.diffuseColor.x * intersection.mtlColor.ambientCoefficient,
            .y = intersection.mtlColor.diffuseColor.y * intersection.mtlColor.ambientCoefficient,
            .z = intersection.mtlColor.diffuseColor.z * intersection.mtlColor.ambientCoefficient,
    };
    Vector3 depthCueingAmbient = (Vector3) {
            .x = scene->depthCueing.color.x * intersection.mtlColor.ambientCoefficient,
            .y = scene->depthCueing.color.y * intersection.mtlColor.ambientCoefficient,
            .z = scene->depthCueing.color.z * intersection.mtlColor.ambientCoefficient,
    };
    Vector3 lightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 depthCueingLightsApplied = (Vector3) {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    if (scene->lightCount > 0) {
        for (int lightIdx = 0; lightIdx < scene->lightCount; lightIdx++) {
            Light light = scene->lights[lightIdx];
            Vector3 lightDirection = light.pointOrDirectional == 1.0f
                                     ? normalize(subtract(light.position, intersection.intersectionPoint))
                                     : normalize(multiply(light.position, -1.0f));
            Vector3 halfwayLightDirection = normalize(add(lightDirection, multiply(intersection.incidentDirection, -1.0f)));
            float lightIntensity = light.intensity;
            Vector3 diffuse = (Vector3) {
                    .x = intersection.mtlColor.diffuseColor.x * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .y = intersection.mtlColor.diffuseColor.y * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .z = intersection.mtlColor.diffuseColor.z * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 depthCueingDiffuse = (Vector3) {
                    .x = scene->depthCueing.color.x * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .y = scene->depthCueing.color.y * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .z = scene->depthCueing.color.z * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 specular = (Vector3) {
                    .x =  intersection.mtlColor.specularColor.x * intersection.mtlColor.specularCoefficient *
                          powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .y = intersection.mtlColor.specularColor.y * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .z = intersection.mtlColor.specularColor.z * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
            };
            Vector3 depthCueingSpecular = (Vector3) {
                    .x =  scene->depthCueing.color.x * intersection.mtlColor.specularCoefficient *
                          powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .y = scene->depthCueing.color.y * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .z = scene->depthCueing.color.z * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
            };

            if (scene->softShadows) {
                float softShadow = 0.0f;
                int numShadowRays = 50;

                Intersection centralShadowIntersection = castRay((Ray) {
                        .origin = intersection.intersectionPoint,
                        .direction = lightDirection
                }, scene, intersection.exclusion);
                for (int i = 0; i < numShadowRays; ++i) {
                    Vector3 jitteredLightDirection = add(lightDirection, multiply(randomUnitVector(), 0.005f));

                    Intersection shadowIntersection = castRay((Ray) {
                            .origin = intersection.intersectionPoint,
                            .direction = normalize(jitteredLightDirection)
                    }, scene, intersection.exclusion);

                    if (shadowIntersection.closestSphereIdx != -1 || shadowIntersection.closestEllipsoidIdx != -1 || shadowIntersection.closestFaceIntersection.faceIdx != -1) {
                        if ((light.pointOrDirectional == 1.0f && shadowIntersection.closestIntersection < distance(intersection.intersectionPoint, light.position)) ||
                            (light.pointOrDirectional == 0.0f && shadowIntersection.closestIntersection > 0.0f)) {
                            softShadow += 1.0f / (float) numShadowRays;
                        }
                    }
                }
                if (centralShadowIntersection.mtlColor.alpha < 1.0f) {
                    softShadow *= (1.0f - centralShadowIntersection.mtlColor.alpha);
                }
                shadow = 1.0f - softShadow;
            } else {
                Intersection shadowIntersection = castRay((Ray) {
                        .origin = intersection.intersectionPoint,
                        .direction = lightDirection
                }, scene, intersection.exclusion);
                if (shadowIntersection.closestSphereIdx != -1 || shadowIntersection.closestEllipsoidIdx != -1 || shadowIntersection.closestFaceIntersection.faceIdx != -1) {
                    if (
                            (light.pointOrDirectional == 1.0f && shadowIntersection.closestIntersection < distance(intersection.intersectionPoint, light.position)) ||
                            (light.pointOrDirectional == 0.0f && shadowIntersection.closestIntersection > 0.0f)
                            ) {
                        shadow *= (1.0f - shadowIntersection.mtlColor.alpha);
                    }
                }
            }

            if (scene->depthCueing.distMax > 0.0f) {
                float distanceToCamera = distance(scene->eye, intersection.intersectionPoint);
                float depthCueFactor = normalizef(distanceToCamera, scene->depthCueing.distMin, scene->depthCueing.distMax);
                diffuse = multiply(diffuse, 1.0f - depthCueFactor);
                specular = multiply(specular, 1.0f - depthCueFactor);
                ambient = multiply(ambient, 1.0f - depthCueFactor);
                depthCueingDiffuse = multiply(depthCueingDiffuse, depthCueFactor);
                depthCueingSpecular = multiply(depthCueingSpecular, depthCueFactor);
                depthCueingAmbient = multiply(depthCueingAmbient, depthCueFactor);
            }

            float attenuationFactor = 1.0f;
            if (light.constantAttenuation > 0.0f || light.linearAttenuation > 0.0f || light.quadraticAttenuation > 0.0f) {
                attenuationFactor = 1.0f / (light.constantAttenuation + light.linearAttenuation * distance(intersection.intersectionPoint, light.position) + light.quadraticAttenuation * powf(distance(
                        intersection.intersectionPoint, light.position), 2.0f));
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
    Vector3 color = clamp(add(ambientApplied, depthCueingAmbientApplied));

    return (Illumination) {
            .ambient = ambientApplied,
            .depthCueingAmbient = depthCueingAmbientApplied,
            .color = color,
    };
}

Reflection applyReflections(
        Scene* scene,
        Intersection intersection,
        Illumination illumination,
        RayState rayState,
        float Fr
) {
    Vector3 baseColor = illumination.color;
    Vector3 reflectionColor = (Vector3) {
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };
    Vector3 reflection = (Vector3) {
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };

    if (intersection.mtlColor.specularCoefficient > 0.0f) {
        reflectionColor = shadeRay(
            reflectRay(
                    intersection.intersectionPoint,
                    multiply(intersection.incidentDirection, -1.0f),
                    intersection.surfaceNormal
                ),
                scene,
                (RayState) {
                    .exclusion = rayState.exclusion,
                    .shadow = rayState.shadow,
                    .reflectionDepth = rayState.reflectionDepth + 1,
                    .previousRefractionIndex = rayState.previousRefractionIndex
                }
        );
        reflection = multiply(reflectionColor, Fr);
        baseColor = clamp(add(
            multiply(illumination.ambient, 1.0f - Fr),
            multiply(illumination.depthCueingAmbient, 1.0f - Fr)
        ));
    }

    if (baseColor.x == 0.0f && baseColor.y == 0.0f && baseColor.z == 0.0f) {
        reflection = (Vector3) {
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f
        };
    }

    Vector3 reflectionsApplied = add(baseColor, reflection);

    return (Reflection) {
        .color = clamp(reflectionsApplied),
        .reflectionColor = clamp(reflectionColor)
    };
}

Vector3 applyTransparency(Scene* scene, Intersection intersection, RayState rayState, Reflection reflection, float intersectionPointReflectionCoefficient, float currentRefractionIndex, float nextRefractionIndex, Exclusion newExclusion) {
    if (intersection.mtlColor.alpha >= 1.0f) {
        return reflection.color;
    }

    Vector3 I = multiply(intersection.incidentDirection, -1.0f);

    float cosThetaEntering = dot(intersection.surfaceNormal, I);
    float refractionCoefficient = currentRefractionIndex / nextRefractionIndex;
    float partUnderSqrt = 1.0f - powf(refractionCoefficient, 2.0f) * (1.0f - powf(cosThetaEntering, 2.0f));
    if (partUnderSqrt < 0.0f) {
        return reflection.reflectionColor;
    }

    Vector3 refractionDirToMultiply = subtract(multiply(intersection.surfaceNormal, cosThetaEntering), I);

    float cosThetaExiting = sqrtf(partUnderSqrt);

    Ray nextIncident = (Ray) {
            .origin = intersection.intersectionPoint,
            .direction = add(
                    multiply(multiply(intersection.surfaceNormal, -1.0f),cosThetaExiting),
                    multiply(refractionDirToMultiply, refractionCoefficient)
            )
    };

    Vector3 transparencyColor = shadeRay(nextIncident, scene, (RayState) {
        .exclusion = rayState.exclusion,
        .shadow = rayState.shadow,
        .reflectionDepth = rayState.reflectionDepth,
        .previousRefractionIndex = currentRefractionIndex
    });

    float distanceTraveled = magnitude(subtract(nextIncident.origin, intersection.intersectionPoint));

    float attenuationCoefficient = intersection.mtlColor.attenuationCoefficient;
    float attenuationFactor = expf(-attenuationCoefficient * distanceTraveled);
    Vector3 attenuatedTransparencyColor = multiply(transparencyColor, attenuationFactor);
    Vector3 transparency = multiply(attenuatedTransparencyColor, (1.0f - intersectionPointReflectionCoefficient) * (1.0f - intersection.mtlColor.alpha));

    return add(reflection.color, transparency);
}

Vector3 applyBlinnPhongIllumination(
        Scene* scene,
        Intersection intersection,
        RayState rayState
) {
    Illumination illumination = applyLights(scene, intersection,  rayState.shadow);

    float currentRefractionIndex = rayState.previousRefractionIndex;
    float nextRefractionIndex = intersection.mtlColor.refractionIndex;

    Exclusion newExclusion = rayState.exclusion;
    bool exiting = dot(intersection.surfaceNormal, intersection.incidentDirection) >= 0;
    if (exiting) {
        float tempRefractionIndex = currentRefractionIndex;
        currentRefractionIndex = nextRefractionIndex;
        nextRefractionIndex = tempRefractionIndex;
        intersection.surfaceNormal = multiply(intersection.surfaceNormal, -1.0f);
        newExclusion = (Exclusion) {
            .excludeSphereIdx = -1,
            .excludeEllipsoidIdx = -1,
            .excludeFaceIdx = intersection.exclusion.excludeFaceIdx
        };
    }

    float F0 = powf(((nextRefractionIndex - currentRefractionIndex) / (nextRefractionIndex + currentRefractionIndex)), 2);
    float Fr = F0 + ((1.0f - F0) * powf(1.0f - dot(multiply(intersection.incidentDirection, -1.0f), intersection.surfaceNormal), 5));
    Reflection reflection = applyReflections(scene, intersection, illumination, rayState, Fr);

    if (exiting && intersection.mtlColor.alpha >= 1.0f) {
        return reflection.color;
    }

    return applyTransparency(scene, intersection, rayState, reflection, Fr, currentRefractionIndex, nextRefractionIndex, newExclusion);
}

Vector3 shadeRay(Ray ray, Scene* scene, RayState rayState) {
    if (rayState.reflectionDepth > MAX_REFLECTION_DEPTH) {
        return scene->bkgColor.color;
    }

    Intersection bvhIntersection = castBvhRay(ray, scene);
    if (scene->bvhSphereCount > 0 && !intersectionExists(bvhIntersection)) {
        return scene->bkgColor.color;
    }

    Intersection intersection = castRay(ray, scene, rayState.exclusion);
    if (!intersectionExists(intersection)) {
        return scene->bkgColor.color;
    }

    return applyBlinnPhongIllumination(scene, intersection, rayState);
}

#endif
