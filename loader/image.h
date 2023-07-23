#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdio.h>
#include <string.h>
#include "../kernel/header.h"

void save_to_file(unsigned char* image_RGBA, char* location, int X_RES, int Y_RES);
void save_to_file(unsigned char* image_RGBA, char* location, int X_RES, int Y_RES) {
    FILE* ppmfile;
    ppmfile = fopen(location, "wb");
    fprintf(ppmfile, "P6\n");
    fprintf(ppmfile, "%d %d\n", X_RES, Y_RES); 
    fprintf(ppmfile, "255\n");

    int out_of_bound = 0;
    int x=0, y=0;
    for (y = Y_RES; y-- ;) {
        for (x = X_RES; x-- ;) {
            unsigned int RED_CHAN = *(image_RGBA + y*X_RES*4 +x*4 +0);
            unsigned int GRE_CHAN = *(image_RGBA + y*X_RES*4 +x*4 +1);    
            unsigned int BLU_CHAN = *(image_RGBA + y*X_RES*4 +x*4 +2);
            if (RED_CHAN>255 || GRE_CHAN>255 || BLU_CHAN>255) {
                out_of_bound++;
            }
            fwrite(image_RGBA + y*X_RES*4 +x*4 +0, 1, 1, ppmfile);
            fwrite(image_RGBA + y*X_RES*4 +x*4 +1, 1, 1, ppmfile);
            fwrite(image_RGBA + y*X_RES*4 +x*4 +2, 1, 1, ppmfile);
        }
    }
    if (out_of_bound)
        printf(" ##### /!\\ Color out of bound (%i) !!! ##### \n", out_of_bound);
    
    printf("Image saved [%s] \n", location);
    fclose(ppmfile);
}


int load_file(char* location, int* x_size, int* y_size, int x_offset, int y_offset, unsigned char** image_RGBA);
int load_file(char* location, int* x_size, int* y_size, int x_offset, int y_offset, unsigned char** image_RGBA) {
    
    FILE* ppmfile = fopen(location, "rb");
    //printf("load file: %s\n", location);

    if (ppmfile == NULL) {
        printf("file not found [%s]\n", location);
        return 0;
    }

    char line[1000];
    char format[10];
    int max_size = 0;


    // Extract magic number
    do {fgets(line, sizeof(line), ppmfile);
    } while(line[0] == '#' || line[0] == '\n');
    sscanf(line, "%s", format);
    //printf("Format: %s\n", format);

    // Extract xres and yres
    do {fgets(line, sizeof(line), ppmfile);
    } while(line[0] == '#' || line[0] == '\n');
    sscanf(line, "%i %i", x_size, y_size);
    //printf("x: %i, y: %i\n", *x_size, *y_size);

    // Extract maxval (should always be 255)
    do {fgets(line, sizeof(line), ppmfile);
    } while(line[0] == '#' || line[0] == '\n');
    sscanf(line, "%d", &max_size);
    //printf("Size: %d\n", max_size);
    float normalizer = (float) 255/max_size;//<- always use 255 based

    *image_RGBA = (unsigned char*) calloc((*x_size) * (*y_size) * 4, sizeof(char));

    if (!strncmp(format, "P6", 2)) {
        int x, y;for (y = *y_size; y-- ;) {
            for (x = *x_size; x-- ;) {
                char xyz[3];
                fread(xyz, 1, 3, ppmfile);
                *(*image_RGBA + y*(*x_size)*4 + (*x_size -x-1)*4 +0) = xyz[0]*normalizer;
                *(*image_RGBA + y*(*x_size)*4 + (*x_size -x-1)*4 +1) = xyz[1]*normalizer;
                *(*image_RGBA + y*(*x_size)*4 + (*x_size -x-1)*4 +2) = xyz[2]*normalizer;
            }
        }
        fclose(ppmfile);
        printf("Image loaded [%s] [%i, %i]\n", location, *x_size, *y_size);
        return 0;
    } else {
        fclose(ppmfile);
        printf("Unsupported format");
        return 1;
    }
    printf("Unsupported format [%s] [%s]\n", location, format);
    return 1;
    
}

#endif // IMAGE_H_
