#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H

#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float z;
} Vector3;

typedef struct {
    Vector3 direction;
    Vector3 origin;
} Ray;

typedef struct {
    float h;
    float v;
} FieldOfView;

typedef struct {
    int width;
    int height;
} ImSize;

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBColor;

typedef struct {
    Vector3 diffuseColor;
    Vector3 specularColor;
    float ambientCoefficient;
    float diffuseCoefficient;
    float specularCoefficient;
    float specularExponent;
} MaterialColor;

typedef struct {
    float frustumWidth;
} Parallel;

typedef struct {
    Vector3 center;
    float radius;
    int mtlColorIdx;
} Sphere;

typedef struct {
    Vector3 center;
    Vector3 radius;
    int mtlColorIdx;
} Ellipsoid;

typedef struct {
    Vector3 position;
    float w;
    float i;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
} Light;

typedef struct {
    Vector3 color;
    float min;
    float max;
    float distMin;
    float distMax;
} DepthCueing;

typedef struct {
    int v1;
    int v2;
    int v3;
    int vt1;
    int vt2;
    int vt3;
    int vn1;
    int vn2;
    int vn3;
    int mtlColorIdx;
} Face;

typedef struct {
    Vector3 eye;
    Vector3 viewDir;
    Vector3 upDir;
    FieldOfView fov;
    ImSize imSize;
    Vector3 bkgColor;
    Parallel parallel;
    MaterialColor* mtlColors;
    int mtlColorCount;
    Sphere* spheres;
    int sphereCount;
    Ellipsoid* ellipsoids;
    int ellipsoidCount;
    Light* lights;
    int lightCount;
    DepthCueing depthCueing;
    bool softShadows;
    Vector3* vertexes;
    int vertexCount;
    Vector3* vertexNormals;
    int vertexNormalCount;
    Face* faces;
    int faceCount;
} Scene;

typedef struct {
    float width;
    float height;
    Vector3 ul;
    Vector3 ur;
    Vector3 ll;
    Vector3 lr;
} ViewingWindow;

typedef struct {
    Vector3 w;
    Vector3 n;
    Vector3 u;
    Vector3 v;
    float aspectRatio;
    float d;
    ViewingWindow viewingWindow;
    Vector3 dh;
    Vector3 dv;
} ViewParameters;

enum ObjectType {
    SPHERE,
    ELLIPSOID,
    TRIANGLE
};

typedef struct {
    int faceIdx;
    Vector3 n;
    float alpha;
    float beta;
    float gamma;
} FaceIntersection;

typedef struct {
    float closestIntersection;
    int closestSphereIdx;
    int closestEllipsoidIdx;
    FaceIntersection closestFaceIntersection;
    enum ObjectType closestObject;
} Intersection;

#endif
