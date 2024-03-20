#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_COLOR_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_COLOR_H

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

void printRGBColor(RGBColor c) {
    printf("{red: %hhu, green: %hhu, blue: %hhu}\n", c.red, c.green, c.blue);
}

#endif