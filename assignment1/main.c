#include "inputhandler.h"

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char*** inputFileWordsByLine = readInputFile(argv[1]);

    VectorOrPoint3D eye = { .x = 0, .y = 0, .z = 0 }; // do we want to default?
    VectorOrPoint3D viewDir = { .x = 0, .y = 0, .z = 0 };
    VectorOrPoint3D upDir = { .x = 0, .y = 0, .z = 0 };
    HorizontalFOV hfov = { .h = 0 };
    ImSize imSize = { .width = 0, .height = 0 };
    RGBColor bkgColor = { .red = 0, .green = 0, .blue = 0 };
    Parallel parallel = { .frustumWidth = 0 };
    MtlColor* mtlColors = NULL;
    int mtlColorCount = 0;

    int line = 0;
    readInputHeader(inputFileWordsByLine, &line, &eye, &viewDir, &upDir, &hfov, &imSize, &bkgColor, &parallel);
    readInputObjects(inputFileWordsByLine, &line, &mtlColors, &mtlColorCount);

    freeInputFileWordsByLine(inputFileWordsByLine);

    printInput(eye, viewDir, upDir, hfov, imSize, bkgColor, parallel, mtlColors, mtlColorCount);

    freeInput(mtlColors, mtlColorCount);
    exit(0);
}
