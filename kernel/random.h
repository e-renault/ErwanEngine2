#ifndef RANDOM_H_
#define RANDOM_H_

int random(unsigned long globalID, unsigned long* randoms) {
    unsigned int seed = *randoms & globalID;
    unsigned int result = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    *randoms = result;
    return result;
}

float randomf(unsigned int globalID, unsigned int* randoms) {
    unsigned int seed = *randoms & globalID;
    unsigned int result = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    *randoms = result;
    return (float) result/INT_MAX;
}

#endif //RANDOM_H_