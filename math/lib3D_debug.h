#ifndef LIB3D_DEBUG_H_
#define LIB3D_DEBUG_H_

#include "lib3D.h"

void printPoint3(Point3 p) {
    printf("(%f, %f, %f)", p.x, p.y, p.z);
}

void printVector3(Vector3 v) {
    printf("{%f, %f, %f}", v.x, v.y, v.z);
}

void printMatrix3(Matrix3 m) {
    printf("{{%f, %f, %f)},\n"
    " {%f, %f, %f)},\n"
    " {%f, %f, %f)}}\n", 
    m.mat.s[0*4 + 0], m.mat.s[0*4 + 1], m.mat.s[0*4 + 2], 
    m.mat.s[1*4 + 0], m.mat.s[1*4 + 1], m.mat.s[1*4 + 2], 
    m.mat.s[2*4 + 0], m.mat.s[2*4 + 1], m.mat.s[2*4 + 2]);
}

void printRay3(Ray3 r) {
    printf("{p:");
    printPoint3(r.p);
    printf(", v:");
    printVector3(r.v);
    printf("}");
}

void printPlane3(Plane3 p) {
    printf("{n:");
    printVector3(p.n);
    printf(", p:");
    printPoint3(p.p);
    printf(", off: %f", p.off);
    printf("}");
}

void printTriangle3(Triangle3 t) {
    printf("{");
    printf("\n  p:{");
    printf("\n    ");printPoint3(t.p[0]);printf(",");
    printf("\n    ");printPoint3(t.p[1]);printf(",");
    printf("\n    ");printPoint3(t.p[2]);printf(",");
    printf("\n  },");

    printf("\n  v:{");
    printf("\n    ");printVector3(t.v[0]);printf(",");
    printf("\n    ");printVector3(t.v[1]);printf(",");
    printf("\n  },");

    printf("\n  plane:{");
    printf("\n    ");printPlane3(t.pl);
    printf("\n  },");
    
    printf("\n  base:\n");
    printMatrix3(t.base);
    printf("  binv:\n");
    printMatrix3(t.binv);
    printf("}");
}

#endif // LIB3D_DEBUG_H_
