#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdio.h>
#include "color.h"

typedef struct frameRGB {
    int x_res, y_res;
    rgb* frame;
} frameRGB;

frameRGB newFrame(int x_res, int y_res) {
    frameRGB ret;
    ret.x_res = x_res;
    ret.y_res = y_res;

    ret.frame = (rgb*) calloc(x_res * y_res, sizeof(rgb));

    int x;for (x=0; x<x_res; x++) {
        int y;for (y=0; y<y_res; y++) {
            ret.frame[y* x_res + x].r = 0;
            ret.frame[y* x_res + x].g = 0;
            ret.frame[y* x_res + x].b = 0;
        }
    }
    return ret;
}

void generateImg(frameRGB image, char* location);
void generateImg(frameRGB image, char* location) {
    FILE* ppmfile;
    //TODO: file name based on houre
    char loc[100];
    sprintf(loc, "_output/%s.ppm", location);
    ppmfile = fopen(loc, "wb");
    fprintf(ppmfile, "P3\n"); // Writing Magic Number to the File
    fprintf(ppmfile, "%d %d\n", image.x_res, image.y_res); 
    fprintf(ppmfile, "255\n"); // Writing the maximum value

    int out_of_bound = 0;

    int y = image.y_res-1;for (; y-- ;) {
        int x;for (x = 0; x < image.x_res; x++) {
            rgb c = image.frame[y * image.x_res + x];
            int r = (c.r *255);
            int g = (c.g *255);
            int b = (c.b *255);

            if (r>255 || g>255 || b>255 || r<0 || g<0 || b<0) {
                out_of_bound++;
                //printf("%i %i %i\n" ,r, g, b);
            }

            r = r>255 ? 255 : r;
            g = g>255 ? 255 : g;
            b = b>255 ? 255 : b;

            r = r<0 ? 0 : r;
            g = g<0 ? 0 : g;
            b = b<0 ? 0 : b;

            fprintf(ppmfile, "%d %d %d\n", r, g, b);
        }
    }
    if (out_of_bound)
        printf(" ##### /!\ Color out of bound (%i) !!! ##### \n", out_of_bound);

    fclose(ppmfile);
}

#endif // IMAGE_H_
