#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "../kernel/header.h"

rgb getColor(Texture texture, EE_FLOAT cx, EE_FLOAT cy) {
    int x = cx * texture.x_res;
    int y = cy * texture.y_res;

    rgb cbase = (texture.color1 * (1-(cy+cx)));
    rgb colorx = (texture.color2 * cx);
    rgb colory = (texture.color3 * cy);

    rgb ret = (cbase + colorx);
    ret = (ret + colory);
    
    return ret;
}


#endif //TEXTURE_H_