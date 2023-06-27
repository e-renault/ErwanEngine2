#ifndef RANDOM_H_
#define RANDOM_H_
#include "header.h"

//TODO use unsigned long instead 
int random_int(unsigned int globalID, unsigned int* randoms);
int random_int(unsigned int globalID, unsigned int* randoms) {
    unsigned int seed = *randoms & globalID;
    unsigned int result = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    *randoms = result;
    return result;
}

float random_float(unsigned int globalID, unsigned int* randoms);
float random_float(unsigned int globalID, unsigned int* randoms) {
    unsigned int result = random_int(globalID, randoms);
    return (float) result/EE_MAX_INT;
}

#endif //RANDOM_H_