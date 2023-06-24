#ifndef COLOR_H_
#define COLOR_H_

#include "../kernel/header.h"

//TODO: to be deleted ?
rgb cap(rgb color);
rgb cap(rgb color) {
    return (rgb) {
        color.x>1 ? 1 : color.x<0 ? 0 : color.x,
        color.y>1 ? 1 : color.y<0 ? 0 : color.y,
        color.z>1 ? 1 : color.z<0 ? 0 : color.z,
        1
    };
}

#endif //COLOR_H_