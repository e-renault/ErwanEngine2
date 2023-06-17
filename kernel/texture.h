#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "../kernel/header.h"

//TODO: Deprecated
rgb getColor(Texture texture, EE_FLOAT cx, EE_FLOAT cy) {
    int x = cx * 1;
    int y = cy * 1;

    rgb cbase = (texture.color1 * (1-(cy+cx)));
    rgb colorx = (texture.color2 * cx);
    rgb colory = (texture.color3 * cy);

    rgb ret = (cbase + colorx);
    ret = (ret + colory);
    
    return ret;
}

rgb getColor2(Texture texture, __read_only image2d_t texture_map, EE_FLOAT cx, EE_FLOAT cy) {
    int2 img_dim = get_image_dim(texture_map);

    float2 vf = (cx*texture.v1 + texture.voff)*img_dim.x + (cy*texture.v2 + texture.voff)*img_dim.y;
    int2 coord = (int2)( (int) vf.x %img_dim.x, (int) vf.y %img_dim.y);

    sampler_t msampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;
    uint4 rgbint = read_imageui(texture_map, msampler, coord);

    rgb ret = {(float) rgbint.x/255, (float) rgbint.y/255, (float) rgbint.z/255};
    
    return ret;
}


#endif //TEXTURE_H_