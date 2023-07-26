#ifndef GUI_TOOLS_H_
#define GUI_TOOLS_H_

#include <GL/freeglut.h> //for window
#include "../math/mVector3.h"
#include "../math/lib3D_debug.h"


/**  DISPLAY SECTION  **/

void drawText(const char* text, cl_float2 coord, rgb color);
void drawText(const char* text, cl_float2 coord, rgb color) {
    glRasterPos2f(coord.x, coord.y);
    glColor3f(color.x, color.y, color.z);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *(text++));
    }
}

long long t_microsec;

void display_callback();
void display_callback(){
    if (cam_moved) {
        t_microsec = 0;
    }

    if (new_frame) {
        new_frame--;

        glRasterPos2i(-1, -1);
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );
        glDrawPixels( X_RES, Y_RES, GL_RGBA, GL_UNSIGNED_BYTE, output_render_buffer );


        long long t_sec = stop.tv_sec - start.tv_sec;
        t_microsec += (stop.tv_usec - start.tv_usec) + 1000000*t_sec;
        long long sec = t_microsec/1000000;
        long long milisec = t_microsec%1000000 /1000;
        long long microsec = t_microsec%1000000 % 1000;

        char FPS[500];
        sprintf(FPS, "%llus%llums%lluus           \r", sec, milisec, microsec); fflush(stdout);

        cl_float2 coo1 = {-0.992f, 0.86f};
        rgb dark = (rgb) {0.0f, 0.0f, 0.0f, 1.0f};
        drawText(FPS, coo1, dark);

        cl_float2 coo2 = {-0.992f, 0.94f};
        rgb white = (rgb) {1.0f, 1.0f, 1.0f, 1.0f};
        drawText(FPS, coo2, white);

        glutSwapBuffers();
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int xmouse, int ymouse);
void keyboard(unsigned char key, int xmouse, int ymouse) {
    Vector3 rightVector = crossProduct(cam_lookat, UP);
    switch (key) {
        case 'z':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(0.1, cam_lookat));cam_moved = 1;break;
        case 's':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(-0.1f, cam_lookat));cam_moved = 1;break;
        case 'q':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(-0.1, rightVector));cam_moved = 1;break;
        case 'd':cam_coordinate = add_vector3(cam_coordinate, scale_vector3(0.1f, rightVector));cam_moved = 1;break;
        case ' ':cam_coordinate.y += 0.1;cam_moved = 1;break;
        case 'e':cam_coordinate.y -= 0.1;cam_moved = 1;break;
        case 'm':cam_lookat = rotateAround(cam_lookat, UP, -0.1f);cam_moved = 1;break;
        case 'k':cam_lookat = rotateAround(cam_lookat, UP, 0.1f);cam_moved = 1;break;
        case 'o': if (getAngle(cam_lookat, UP) > 0.2f) cam_lookat = rotateAround(cam_lookat, rightVector, 0.1f);cam_moved = 1;break;
        case 'l': if (getAngle(cam_lookat, DOWN) > 0.2f) cam_lookat = rotateAround(cam_lookat, rightVector, -0.1f);cam_moved = 1;break;
        case 't':sky_light_dir.x +=0.1;scene_changed = 1;break;
        case 'g':sky_light_dir.x -=0.1;scene_changed = 1;break;
        case 'y':sky_light_dir.y +=0.1;scene_changed = 1;break;
        case 'h':sky_light_dir.y -=0.1;scene_changed = 1;break;
        case 'u':sky_light_dir.z +=0.1;scene_changed = 1;break;
        case 'j':sky_light_dir.z -=0.1;scene_changed = 1;break;
        case 'r':enqueueKernel = 1; break;
        case 'x':
            program_running_loop = 0;
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
    if (DISPLAY_SCENE_INFO) {
        printf("\rCam: {pos: ");
        printPoint3(cam_coordinate);
        printf(", dir: ");
        printVector3(cam_lookat);
        printf("} - Sun_light: {");
        printVector3(sky_light_dir);
        printf("}   ");
    }
    sem_post(&mutex);
}

void* start_glut(void *vargp);
void* start_glut(void *vargp) {
    if (DEBUG_GLOBAL_INFO) printf("GLUT Start\n");
    //init GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(X_RES, Y_RES);
    
    //glutGameModeString("800x600:32@60");
    //glutEnterGameMode();
    //glutFullScreen();

    glutCreateWindow("ErwanEngine2.0");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //display loop
    glutDisplayFunc(display_callback);
    glutKeyboardFunc(keyboard);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutMainLoop();
    program_running_loop = 0;
    if (DEBUG_GLOBAL_INFO) printf("GLUT Exit\n");
}

#endif //GUI_TOOLS_H_