#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>

#include "scene/sceneLoader.h"


int main(int argc, char *argv[]) {
    Point3 points[100];
    Triangle3* triangles = (Triangle3*) malloc(100 * sizeof(Triangle3));
    char scene_name[50];
    char object[50];

    loadSceneFromFile("/media/erenault/EXCHANGE/workspaces/ErwanEngine2/opencl/scene/scene1.obj", points, triangles, scene_name, object);
    
    free(triangles);

    return 1;
}