#ifndef COLOR_H_
#define COLOR_H_

#include "../kernel/header.h"

typedef struct __attribute__ ((packed)) rgb {
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

rgb min_color(rgb c1, rgb c2) {
    return (rgb) {
        .r=c1.r < c2.r ? c1.r : c2.r,
        .g=c1.g < c2.g ? c1.g : c2.g,
        .b=c1.b < c2.b ? c1.b : c2.b
    };
}

rgb cap(rgb color) {
    float r = color.r>1 ? 1 : color.r;
    float g = color.g>1 ? 1 : color.g;
    float b = color.b>1 ? 1 : color.b;
    
    r = color.r<0 ? 0 : color.r;
    g = color.g<0 ? 0 : color.g;
    b = color.b<0 ? 0 : color.b;

    return (rgb) {
        .r=1,
        .g=g,
        .b=b
    };
}

#endif //COLOR_H_