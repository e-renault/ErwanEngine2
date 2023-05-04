// to compile : gcc testGraphic.c -o demo -lglut -lGL
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>

const unsigned int W = 800;
const unsigned int H = 600;
float* sharedData;//displayed data buffer
int frame, time, timebase;//used for FPS counter

//setup datas
void setup() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void display(){
    glClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT );

    //recover data
    float* data = sharedData;

    //FPS Counter
    frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
        printf("FPS:%4.2f\n", frame*1000.0/(time-timebase));
        fflush(stdout);
	 	timebase = time;
		frame = 0;
    }


    //draw pixels
    glDrawPixels( W, H, GL_RGB, GL_FLOAT, data );
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char *argv[]) {
    //init GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(W,H);
    glutCreateWindow("Hello World");
    
    //init var
    setup();
    frame=0;
    time=0;
    timebase=0;

    //recover data
    int x, y;
    float data[H][W][3];
    for( y = 0; y < H; ++y ) {
        for( x = 0; x < W; ++x ) {
            int r = ((x<200)) ? 1: 0;
            data[y][x][0] = ( r );
            data[y][x][1] = ( r );
            data[y][x][2] = ( r );
        }
    }
    sharedData = data;

    //display loop
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
