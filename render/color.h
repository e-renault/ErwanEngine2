#ifndef COLOR_H_
#define COLOR_H_

#include "../kernel/header.h"

typedef struct rgb {
    EE_FLOAT r, g, b;
} rgb;

//sumup: color->white
rgb synthese_additive(rgb c1, rgb c2) {
    return (rgb) {
        .r=c1.r + c2.r,
        .g=c1.g + c2.g,
        .b=c1.b + c2.b
    };
}

//scale -> 0: color->black;  scale -> 1: color->color
rgb scaling_substractive(rgb color, EE_FLOAT scale) {
    return (rgb) {
        .r=color.r * scale,
        .g=color.g * scale,
        .b=color.b * scale
    };
}

#endif //COLOR_H_