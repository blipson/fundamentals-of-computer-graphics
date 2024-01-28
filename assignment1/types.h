#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H

typedef struct {
    double x;
    double y;
    double z;
} Vector3;

// should I have a Point3???

typedef struct {
    Vector3 direction;
    Vector3 origin;
} Ray;

typedef struct {
    double h;
    double v;
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
    int mtlColorIdx;
} Sphere;

typedef struct {
    Vector3 center;
    Vector3 radius;
    int mtlColorIdx;
} Ellipse;

typedef struct {
    Vector3 eye;
    Vector3 viewDir;
    Vector3 upDir;
    FieldOfView fov;
    ImSize imSize;
    RGBColor bkgColor;
    Parallel parallel;
    RGBColor* mtlColors;
    int mtlColorCount;
    Sphere* spheres;
    int sphereCount;
    Ellipse* ellipses;
    int ellipseCount;
} Scene;

#endif
