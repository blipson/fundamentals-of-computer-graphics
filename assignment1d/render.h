#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define EPSILON 1e-2

#include <float.h>
#include "vector.h"
#include "ray.h"

RGBColor shadeRay(Ray ray, Scene scene, Exclusion exclusion, int reflectionDepth, float shadow, bool entering);

Illumination applyLights(Scene scene, Intersection intersection, float shadow) {
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
                    .x = intersection.mtlColor.diffuseColor.x * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .y = intersection.mtlColor.diffuseColor.y * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .z = intersection.mtlColor.diffuseColor.z * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
            };
            Vector3 depthCueingDiffuse = (Vector3) {
                    .x = scene.depthCueing.color.x * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .y = scene.depthCueing.color.y * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
                    .z = scene.depthCueing.color.z * intersection.mtlColor.diffuseCoefficient * max(dot(intersection.surfaceNormal, lightDirection), 0.0f),
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
                    .x =  scene.depthCueing.color.x * intersection.mtlColor.specularCoefficient *
                          powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .y = scene.depthCueing.color.y * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
                    .z = scene.depthCueing.color.z * intersection.mtlColor.specularCoefficient *
                         powf(max(dot(intersection.surfaceNormal, halfwayLightDirection), 0.0f), intersection.mtlColor.specularExponent),
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
                    }, scene, intersection.exclusion);

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
                }, scene, intersection.exclusion);
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
                ambient = multiply(ambient, 1.0f - depthCueFactor);
                depthCueingDiffuse = multiply(depthCueingDiffuse, depthCueFactor);
                depthCueingSpecular = multiply(depthCueingSpecular, depthCueFactor);
                depthCueingAmbient = multiply(depthCueingAmbient, depthCueFactor);
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
            lightsApplied = add(lightsApplied, shadowsApplied);
            depthCueingLightsApplied = add(lightsApplied, depthCueingShadowsApplied);
        }
    }
    return (Illumination) {
        .ambient = ambient,
        .depthCueingAmbient = depthCueingAmbient,
        .color = lightsApplied,
        .depthCueingColor = depthCueingLightsApplied
    };
}

Reflection applyReflections(
        Scene scene,
        Intersection intersection,
        Illumination lightsApplied,
        float shadow,
        int reflectionDepth,
        bool entering,
        float Fr
) {
    Vector3 ambientApplied = add(lightsApplied.ambient, lightsApplied.color);
    Vector3 depthCueingAmbientApplied = add(lightsApplied.depthCueingAmbient, lightsApplied.depthCueingColor);
    Vector3 baseColor = add(ambientApplied, depthCueingAmbientApplied);

    Vector3 reflectionColor = baseColor;

    Vector3 reflection = (Vector3) {
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f
    };

    // reflections
    if (intersection.mtlColor.specularCoefficient > 0.0f) {
        reflectionColor = convertRGBColorToColor(shadeRay(reflectRay(intersection.intersectionPoint, multiply(intersection.incidentDirection, -1.0f), intersection.surfaceNormal), scene, intersection.exclusion, reflectionDepth + 1, shadow, entering));
        reflection = multiply(reflectionColor, Fr);
        ambientApplied = multiply(ambientApplied, 1.0f - Fr);
        depthCueingAmbientApplied = multiply(depthCueingAmbientApplied, 1.0f - Fr);
        baseColor = add(ambientApplied, depthCueingAmbientApplied);
    }

    Vector3 clampedIllumination = clamp(baseColor);

    if (clampedIllumination.x == 0.0f && clampedIllumination.y == 0.0f && clampedIllumination.z == 0.0f) {
        reflection = (Vector3) {
                .x = 0.0f,
                .y = 0.0f,
                .z = 0.0f
        };
    }

    Vector3 illumination = add(clampedIllumination, reflection);

    return (Reflection) {
        .color = illumination,
        .reflectionColor = reflectionColor
    };
}

Vector3 applyBlinnPhongIllumination(
        Scene scene,
        Intersection intersection,
        int reflectionDepth,
        float shadow,
        bool entering
) {

    Illumination lightsApplied = applyLights(scene, intersection, shadow);

    float F0 = powf(((intersection.mtlColor.refractionIndex - 1.0f) / (intersection.mtlColor.refractionIndex + 1.0f)), 2);
    float Fr = F0 + ((1.0f - F0) * powf(1.0f - dot(multiply(intersection.incidentDirection, -1.0f), intersection.surfaceNormal), 5));
    Reflection reflection = applyReflections(scene, intersection, lightsApplied, shadow, reflectionDepth, entering, Fr);

    // refraction
    if (intersection.mtlColor.alpha >= 1.0f) {
        return reflection.color;
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
        return reflection.reflectionColor;
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

    RGBColor refractionColor = shadeRay(nextIncident, scene, intersection.exclusion, reflectionDepth, shadow, !entering);

    Vector3 transparency = multiply(convertRGBColorToColor(refractionColor), ((1.0f - Fr) * (1.0f - intersection.mtlColor.alpha)));
    return add(reflection.color, transparency);

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
