#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "../kernel/header.h"

typedef struct __attribute__ ((packed)) Texture {
    EE_INT x_res;
    EE_INT y_res;
    rgb color1;
    rgb color2;
    rgb color3;
} Texture;

rgb getColor(Texture texture, EE_FLOAT cx, EE_FLOAT cy) {
    int x = cx * texture.x_res;
    int y = cy * texture.y_res;

    rgb cbase = scaling_substractive(texture.color1, ((1-cx) + (1-cy))/2);
    rgb colorx = scaling_substractive(texture.color2, cx);
    rgb colory = scaling_substractive(texture.color3, cy);

    rgb ret = synthese_additive(cbase, colorx);
    ret = synthese_additive(ret, colory);
    
    return ret;
}


#endif //TEXTURE_H_