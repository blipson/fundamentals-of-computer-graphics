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
    float alpha;
    float refractionIndex;
} MaterialColor;

typedef struct {
    float frustumWidth;
} Parallel;

typedef struct {
    Vector3 center;
    float radius;
    int mtlColorIdx;
    int textureIdx;
    int normalIdx;
} Sphere;

typedef struct {
    Vector3 center;
    Vector3 radius;
    int mtlColorIdx;
    int textureIdx;
    int normalIdx;
} Ellipsoid;

typedef struct {
    Vector3 position;
    float pointOrDirectional;
    float intensity;
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
    int textureIdx;
    int normalIdx;
} Face;

typedef struct {
    int width;
    int height;
    int maxColor;
    RGBColor** data;
} PPMImage;

typedef struct {
    float u;
    float v;
} TextureCoordinate;

typedef struct {
    Vector3 color;
    float refractionIndex;
} Background;

typedef struct {
    Vector3 eye;
    Vector3 viewDir;
    Vector3 upDir;
    FieldOfView fov;
    ImSize imSize;
    Background bkgColor;
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
    TextureCoordinate* vertexTextures;
    int vertexTextureCount;
    Face* faces;
    int faceCount;
    PPMImage* textures;
    int textureCount;
    PPMImage* normals;
    int normalCount;
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
    Vector3 normalDirection;
    float alpha;
    float beta;
    float gamma;
    Vector3 tangentDirection;
    Vector3 bitangentDirection;
} FaceIntersection;

typedef struct {
    int excludeSphereIdx;
    int excludeEllipsoidIdx;
    int excludeFaceIdx;
} Exclusion;

typedef struct {
    float closestIntersection;
    int closestSphereIdx;
    int closestEllipsoidIdx;
    FaceIntersection closestFaceIntersection;
    enum ObjectType closestObject;
    Vector3 intersectionPoint;
    Vector3 surfaceNormal;
    Vector3 incidentDirection;
    MaterialColor mtlColor;
    Exclusion exclusion;
} Intersection;

typedef struct {
    Vector3 color;
    Vector3 reflectionColor;
} Reflection;

typedef struct {
    Vector3 ambient;
    Vector3 depthCueingAmbient;
    Vector3 color;
} Illumination;

typedef struct {
    Exclusion exclusion;
    int reflectionDepth;
    float shadow;
} RayState;

#endif
