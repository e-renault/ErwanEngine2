#ifndef GUI_TOOLS_H_
#define GUI_TOOLS_H_

#include <GL/freeglut.h> //for window
#include "math/mVector3.h"
#include "math/lib3D_debug.h"


/**  DISPLAY SECTION  **/
void display_callback();
void display_callback(){
    if (new_frame) {
        new_frame=0;
        
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );

        //draw pixels
        glDrawPixels( X_RES, Y_RES, GL_RGBA, GL_UNSIGNED_BYTE, output_render_buffer );
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
        case 'r':cube_demo = !cube_demo; break;
        case 'x':
            program_running_loop = 0;
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
    if (SHOW_FPS) {
        printf("\rCam: {pos: ");
        printPoint3(cam_coordinate);
        printf(", dir: ");
        printVector3(cam_lookat);
        printf("} - Sun_light: {");
        printVector3(sky_light_dir);
        printf("}   ");
    }
}

void* start_glut(void *vargp);
void* start_glut(void *vargp) {
    if (DEBUG_GLOBAL_INFO) printf("GLUT Start\n");
    //init GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(X_RES, Y_RES);
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