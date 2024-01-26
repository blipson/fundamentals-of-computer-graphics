#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H

typedef struct {
    int x;
    int y;
    int z;
} VectorOrPoint3D;

typedef struct {
    int h;
} HorizontalFOV;

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
    int frustumWidth;
} Parallel;

typedef struct {
    int x;
    int y;
    int z;
    int r;
} Sphere;

typedef struct {
    int cx;
    int cy;
    int cz;
    int rx;
    int ry;
    int rz;
} Ellipse;

typedef struct {
    RGBColor color;
    Sphere* spheres;
    int sphereCount;
    Ellipse* ellipses;
    int ellipseCount;
} MtlColor;

#endif
