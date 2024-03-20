#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_VECTOR_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_VECTOR_H

float distance(Vector3 v1, Vector3 v2) {
    return sqrtf(powf(v2.x - v1.x, 2.0f) + powf(v2.y - v1.y, 2.0f) + powf(v2.z - v1.z, 2.0f));
}

float magnitude(Vector3 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vector3 normalize(Vector3 v) {
    float m = magnitude(v);
    if (m != 0.0f) {
        return (Vector3) {
                .x = v.x / m,
                .y = v.y / m,
                .z = v.z / m
        };
    } else {
        return v;
    }
}

float normalizef(float value, float min, float max) {
    return value / (max - min);
}

Vector3 clamp(Vector3 v) {
    return (Vector3) {
            .x = max(min(v.x, 1.0f), 0.0f),
            .y = max(min(v.y, 1.0f), 0.0f),
            .z = max(min(v.z, 1.0f), 0.0f)
    };
}


Vector3 multiply(Vector3 v, float s) {
    return (Vector3) {
            .x = v.x * s,
            .y = v.y * s,
            .z = v.z * s
    };
}

Vector3 cross(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = (a.y * b.z) - (b.y * a.z),
            .y = (a.z * b.x) - (b.z * a.x),
            .z = (a.x * b.y) - (b.x * a.y)
    };
}

float dot(Vector3 a, Vector3 b) {
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

Vector3 add(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x + b.x,
            .y = a.y + b.y,
            .z = a.z + b.z
    };
}

Vector3 addf(Vector3 v, float s) {
    return (Vector3) {
            .x = v.x + s,
            .y = v.y + s,
            .z = v.z + s
    };
}

Vector3 subtract(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x - b.x,
            .y = a.y - b.y,
            .z = a.z - b.z
    };
}

Vector3 divide(Vector3 v, float c) {
    return (Vector3) {
            .x = v.x / c,
            .y = v.y / c,
            .z = v.z / c
    };
}

Vector3 convertNormalToVector(RGBColor rgbColor) {
    if (rgbColor.blue < 128) {
        rgbColor.blue = 120;
    }
    return (Vector3) {
            .x = ((float) rgbColor.red - 127.5f) / 127.5f,
            .y = ((float) rgbColor.green - 127.5f) / 127.5f,
            .z = (float) rgbColor.blue / 127.5f,
    };
}

float randomFloat() {
    return ((float) rand() / (float) RAND_MAX);
}

Vector3 randomUnitVector() {
    float z = 1.0f - 2.0f * randomFloat();
    float phi = 2.0f * (float) M_PI * randomFloat();
    float sqrtOneMinusZSquared = sqrtf(1.0f - z * z);

    float x = sqrtOneMinusZSquared * cosf(phi);
    float y = sqrtOneMinusZSquared * sinf(phi);

    return (Vector3) {x, y, z};
}

Vector3 tangentSpaceToWorldSpace(Vector3 transformationMatrix, Vector3 tangentDirection, Vector3 bitangentDirection, Vector3 surfaceNormal) {
    return (Vector3) {
            .x = (tangentDirection.x * transformationMatrix.x) + (bitangentDirection.x * transformationMatrix.y) + (surfaceNormal.x * transformationMatrix.z),
            .y = (tangentDirection.y * transformationMatrix.x) + (bitangentDirection.y * transformationMatrix.y) + (surfaceNormal.y * transformationMatrix.z),
            .z = (tangentDirection.z * transformationMatrix.x) + (bitangentDirection.z * transformationMatrix.y) + (surfaceNormal.z * transformationMatrix.z),
    };
}

void printVector(Vector3 v) {
    printf("{x: %lf, y: %lf, z: %lf}\n", v.x, v.y, v.z);
}

#endif