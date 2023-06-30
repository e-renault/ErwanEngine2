exec: compile
	./a.out --XRES 1000 --YRES 800 --FOV 80 --obj default2.obj --texture default_texture.ppm

compile:
	gcc main.c -lm -lOpenCL -lpthread -lGL -lglut
	echo "Compilation Done"

clean:
	rm a.out
