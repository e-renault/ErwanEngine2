#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "../kernel/header.h"

rgb getColor(UV texture, Material mtl, __read_only image2d_t texture_map, EE_FLOAT cx, EE_FLOAT cy) {
    int2 img_dim = get_image_dim(texture_map);


    float2 vf = cx*texture.v1 + cy*texture.v2 + texture.voff;
    int2 coord = (int2)( 
        (int) (vf.x *mtl.map_location.v_size.x +mtl.map_location.v_off.x), 
        (int) (vf.y *mtl.map_location.v_size.y +mtl.map_location.v_off.y) 
    );

    sampler_t msampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
    uint4 rgbint = read_imageui(texture_map, msampler, coord);

    rgb ret = {(float) rgbint.x/255, (float) rgbint.y/255, (float) rgbint.z/255, 0};
    
    return ret;
}


#endif //TEXTURE_H_