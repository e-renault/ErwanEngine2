#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>

#include "math/lib3D.h"
#include "math/lib3D_debug.h"
#include "render/image.h"


int main(int argc, char *argv[]) {
    printf("\n\n ### Point ### \n\n");

    Point3 p1 = {0.5F, 1.25F, 2.125F};
    Point3 p2 = {10.5, -5.25, 100.125};
    Point3 p3 = {9.5, 1.25, 2.125};
    printPoint3(p1);printf("\n");
    printPoint3(p2);printf("\n");
    printPoint3(p3);printf("\n");

    float l0 = getLength(p1, p2);
    printf("[p1, p2]: %f\n", l0);


    printf("\n\n ### Vector ### \n\n");

    Vector3 v1 = {3, -3, 1};
    Vector3 v2 = {4, 9, 2};
    Vector3 v3 = {-1, 0, 2};
    printVector3(v1);printf("\n");
    printVector3(v2);printf("\n");
    printVector3(v3);printf("\n");

    Vector3 v4 = newVector(p1, p2);
    printf("(p1, p2): ");printVector3(v4);printf("\n");

    Vector3 v5 = crossProduct(v1, v2);//−15, −2, 39
    printf("(v1 ^ v2): ");printVector3(v5);printf("\n");

    float l1 = getLength2(v1);//4.35889894354
    printf("[v1]: %f\n", l1);

    Vector3 v6 = getNorm(v1);
    printf("norm(v1): ");printVector3(v6);printf("\n");

    Vector3 v1b = {1, -3, 0};
    Vector3 v2b = {4, 5.6, 0};
    Vector3 v7 = getNorm2(v1b, v2b);
    printf("norm(v1b, v2b): ");printVector3(v7);printf("\n");


    printf("\n\n ### Matrix ### \n\n");

    Matrix3 m1 = {1, 2, -1, 2, 1, 2, -1, 2, 1};
    
    printf("m1:\n");
    printMatrix3(m1);printf("\n");

    printf("inv_m1:\n");
    Matrix3 m4 = invert(m1);
    printMatrix3(m4);printf("\n");

    printf("v1c * m1: \n");
    Vector3 v1c = {0,1,0};
    Vector3 v8 =  multiply(m1, v1c);
    printVector3(v8);printf("\n");
    


    printf("\n\n ### Surfaces ### \n\n");
    Point3 p2d = {1, -2, 0};
    Point3 p3d = {3, 1, 4};
    Point3 p4d = {0, -1, 2};

    Vector3 v1d = newVector(p2d, p3d);
    Vector3 v2d = newVector(p2d, p4d);
    Plane3 pl1 = newPlane3(v1d, v2d, p2d);

    printf("pl1:\n");
    printPlane3(pl1);printf("\n");
    
    Triangle3 t1 = newTriangle3(p2d, p3d, p4d);
    printf("triangle 1:\n");
    printTriangle3(t1);printf("\n");

    Vector3* off1 = (Vector3*) &t1.pl.p;

    Vector3 off2 = minus_vector3(*off1);

    Vector3* hp2d = (Vector3*) &p2d;
    Vector3* hp3d = (Vector3*) &p3d;
    Vector3* hp4d = (Vector3*) &p4d;

    printf("triangle corner in triangle 1:\n");
    printVector3(multiply(t1.base, add_vector3(*hp2d, off2)));printf(",\n");
    printVector3(multiply(t1.base, add_vector3(*hp3d, off2)));printf(",\n");
    printVector3(multiply(t1.base, add_vector3(*hp4d, off2)));printf(",\n");

    printf("\n");printf("\n");
    Plane3 pl2 = {0, 0, 0, 2, 3, -1, -7}; 
    Ray3 r1 = {1, 2, 3, 2, 3, -1};
    printPlane3(pl2);printf(",\n");
    printRay3(r1);printf(",\n");
    Point3 col1 = collisionRayPlane(pl2, r1);

    Vector3* hp2db = (Vector3*) &col1;
    printVector3(*hp2db);printf("\n");

    int x_res = 3, y_res = 3;
    frameRGB image = newFrame(x_res, y_res, 255);
    setPixel(image, 1, 1, 0.5f);
    
    generateImg(image, "color_test");

    return 1;
}