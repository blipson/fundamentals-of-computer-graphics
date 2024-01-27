#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H

typedef struct {
    double x;
    double y;
    double z;
} Vector3;

typedef struct {
    double h;
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
    double frustumWidth;
} Parallel;

typedef struct {
    Vector3 center;
    double radius;
} Sphere;

typedef struct {
    Vector3 center;
    Vector3 radius;
} Ellipse;

typedef struct {
    RGBColor color;
    Sphere* spheres;
    int sphereCount;
    Ellipse* ellipses;
    int ellipseCount;
} MtlColor;

typedef struct {
    Vector3 eye;
    Vector3 viewDir;
    Vector3 upDir;
    FieldOfView fov;
    ImSize imSize;
    RGBColor bkgColor;
    Parallel parallel;
    MtlColor* mtlColors;
    int mtlColorCount;
} Scene;

#endif
